/***********************************************************************************************
 * 版权声明：
 * 本源代码的版权归 [saisaiwa] 所有。
 *
 * 未经版权所有者明确授权，不得将本代码的任何部分用于商业用途，包括但不限于出售、出租、许可或发布。
 * 仅限个人学习、研究、非盈利性用途下使用。如果您有其他用途的需求，请联系
 *[yustart@foxmail.com] 进行授权。
 *
 * 在遵循以下条件的情况下，您可以自由修改、使用和分发本代码：
 * - 您必须保留此版权声明的所有内容。
 * - 您不得用于任何违法、滥用、恶意的活动。
 * - 您的使用不应损害任何个人、组织或作者的声誉。
 *
 * 作者不承担因使用本代码而产生的任何担保责任。作者对因使用本代码所导致的任何直接或间接损失不承担责任。
 * Github: https://github.com/ccy-studio/CCY-VFD-7BT317NK
 ***********************************************************************************************/

#include <ArduinoJson.h>
#include <web_index_html.h>
#include <web_server.h>

ESP8266WebServer server(80);

void handle_index();
void handle_404();
void http_reqeust_save_setteing();
void http_reqeust_get_setteing();

void web_setup() {
    server.begin();
    server.on("/", handle_index);
    server.on("/save-setting", http_reqeust_save_setteing);
    server.on("/get-setting", http_reqeust_get_setteing);
    server.onNotFound(handle_404);
}

void web_stop() {
    server.close();
}

void web_loop() {
    if (WiFi.isConnected()) {
        server.handleClient();
    }
}

void handle_index() {
    String str = FPSTR(HTML_INDEX);
    server.send(200, "text/html; charset=utf-8", str);
}
void handle_404() {
    String html = "<h1>404</h1>";
    server.send(404, "text/html; charset=utf-8", html);
}

void http_reqeust_save_setteing() {
    String body = server.arg("plain");  // 获取POST请求的内容

#ifdef DEBUG
    Serial.println("Received POST data: " + body);
#endif

    server.send(200, "application/json", "success");
}

void http_reqeust_get_setteing() {
    DynamicJsonDocument jsonDoc(1024);
    String jsonResponse;
    serializeJson(jsonDoc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}