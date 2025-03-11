#include "ai_manager.h"
#include "animator.h"
#include "gui.h"

// Constructor
AiManager::AiManager(Animator *animator, const char *apiKey)
    : _animator(animator), _apiKey(apiKey)
{
}

// Destructor
AiManager::~AiManager()
{
    resetState();
}

void AiManager::begin()
{
    // Nothing to do here for now
}

bool AiManager::isActive() const
{
    return _state != State::IDLE && _state != State::COMPLETE && _state != State::ERROR;
}

void AiManager::onComplete(void (*callback)(const String &message))
{
    _completeCallback = callback;
}

void AiManager::onError(void (*callback)(const String &errorMessage))
{
    _errorCallback = callback;
}

void AiManager::startConversation(const char *prompt)
{
    if (isActive())
    {
        return; // Already active
    }

    _prompt = prompt;
    vfd_gui_set_text(" denke");
    _animator->start_loading(0x01);
    _stateStartTime = millis();

    // Initialize HTTP client
    _client = new WiFiClientSecure();
    _http = new HTTPClient();
    _client->setInsecure(); // Only for development
    _doc = new JsonDocument();
    _state = State::CREATE_THREAD;
}

void AiManager::process()
{
    if (_state == State::IDLE)
    {
        return;
    }

    // Check for timeout in any active state
    if (_state != State::COMPLETE && _state != State::ERROR &&
        millis() - _stateStartTime > _aiTimeout)
    {
        handleError("AI operation timed out");
        return;
    }

    // Constants for API endpoints
    const char *BASE_URL = "https://api.openai.com/v1";

    // State machine
    switch (_state)
    {
    case State::CREATE_THREAD:
    {
        _http->useHTTP10(true);
        _http->begin(*_client, String(BASE_URL) + "/threads");
        setHeaders();

        int httpResponseCode = _http->POST("{}");
        if (httpResponseCode <= 0)
        {
            handleError("Error creating thread: " + String(httpResponseCode));
            return;
        }

        // Parse thread ID
        JsonDocument idFilter;
        idFilter["id"] = true;
        deserializeJson(*_doc, _http->getStream(), DeserializationOption::Filter(idFilter));
        _threadId = (*_doc)["id"].as<String>();
        Serial.println("Thread ID: " + _threadId);
        _http->end();

        _state = State::ADD_MESSAGE;
        break;
    }

    case State::ADD_MESSAGE:
    {
        _http->begin(*_client, String(BASE_URL) + "/threads/" + _threadId + "/messages");
        setHeaders();

        String messageJson = "{\"role\":\"user\",\"content\":\"" + _prompt + "\"}";
        int httpResponseCode = _http->POST(messageJson);
        if (httpResponseCode != 200)
        {
            handleError("Error adding message: " + String(httpResponseCode));
            return;
        }
        _http->end();

        _state = State::CREATE_RUN;
        break;
    }

    case State::CREATE_RUN:
    {
        String runEndpoint = String(BASE_URL) + "/threads/" + _threadId + "/runs";
        _http->useHTTP10(true);
        _http->begin(*_client, runEndpoint);
        setHeaders();

        String requestData = "{\"assistant_id\":\"" + _assistantId + "\",\"instructions\":\"\"}";
        int httpResponseCode = _http->POST(requestData);
        if (httpResponseCode != 200)
        {
            handleError("Error creating run: " + String(httpResponseCode));
            return;
        }
        JsonDocument idFilter;
        idFilter["id"] = true;
        deserializeJson(*_doc, _http->getStream(), DeserializationOption::Filter(idFilter));
        _runId = (*_doc)["id"].as<String>();
        Serial.println("Run ID: " + _runId);
        _http->end();

        _state = State::POLL_RUN;
        _lastPollTime = 0; // Initialize for first poll
        break;
    }

    case State::POLL_RUN:
    {
        // We'll poll once every 300ms
        if (millis() - _lastPollTime < 300)
        {
            return; // Wait before polling again
        }
        _lastPollTime = millis();

        String runEndpoint = String(BASE_URL) + "/threads/" + _threadId + "/runs";
        _http->useHTTP10(true);
        _http->begin(*_client, runEndpoint + "/" + _runId);
        setHeaders();

        int httpResponseCode = _http->GET();
        if (httpResponseCode != 200)
        {
            handleError("Error getting run status: " + String(httpResponseCode));
            return;
        }
        JsonDocument statusFilter;
        statusFilter["status"] = true;
        deserializeJson(*_doc, _http->getStream(), DeserializationOption::Filter(statusFilter));
        _http->end();

        String status = (*_doc)["status"].as<String>();
        if (status == "completed")
        {
            _state = State::GET_MESSAGES;
        }
        else if (status == "failed" || status == "cancelled")
        {
            handleError("Run ended with status: " + status);
        }
        break;
    }

    case State::GET_MESSAGES:
    {
        _http->useHTTP10(true);
        _http->begin(*_client, String(BASE_URL) + "/threads/" + _threadId + "/messages");
        setHeaders();

        int httpResponseCode = _http->GET();
        if (httpResponseCode != 200)
        {
            handleError("Error getting messages: " + String(httpResponseCode));
            return;
        }

        JsonDocument msgFilter;
        msgFilter["data"][0]["content"][0]["text"]["value"] = true;
        deserializeJson(*_doc, _http->getStream(), DeserializationOption::Filter(msgFilter));
        _http->end();

        String msg = (*_doc)["data"][0]["content"][0]["text"]["value"].as<String>();
        String processedMsg = processMessage(msg);

        // Display the message
        _animator->stop();
        vfd_gui_set_pic(PIC_PLAY, true);
        _animator->set_text_and_run(processedMsg.c_str(), 210);

        // Call the callback if set
        if (_completeCallback)
        {
            _completeCallback(processedMsg);
        }

        _state = State::COMPLETE;
        break;
    }
    case State::COMPLETE:
        resetState();
        break;
    case State::ERROR:
        resetState();
        break;
    case State::IDLE:
        // Should never happen here
        break;
    }
}

void AiManager::resetState()
{
    if (_http)
    {
        _http->end();
        delete _http;
        _http = nullptr;
    }
    if (_client)
    {
        delete _client;
        _client = nullptr;
    }
    if (_doc)
    {
        delete _doc;
        _doc = nullptr;
    }
    _state = State::IDLE;
    _threadId = "";
    _runId = "";
    _prompt = "";
}

void AiManager::setHeaders()
{
    _http->addHeader("Content-Type", "application/json");
    _http->addHeader("Authorization", "Bearer " + _apiKey);
    _http->addHeader("OpenAI-Beta", "assistants=v2");
}

void AiManager::handleError(const String &errorMessage)
{
    Serial.println(errorMessage);
    _animator->stop();
    vfd_gui_set_text("AI ERR");

    // Call the error callback if set
    if (_errorCallback)
    {
        _errorCallback(errorMessage);
    }

    _state = State::ERROR;
}

String AiManager::processMessage(const String &message)
{
    String result = message;

    // Process character conversion (German umlauts)
    const char *replacements[][2] = {
        {"ä", "ae"}, {"ö", "oe"}, {"ü", "ue"}, {"Ä", "Ae"}, {"Ö", "Oe"}, {"Ü", "Ue"}, {"ß", "ss"}};

    for (auto &pair : replacements)
    {
        result.replace(pair[0], pair[1]);
    }

    return result;
}