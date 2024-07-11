/*
MIT License

Copyright (c) 2020 Rupak Poddar

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ESP8266Firebase.hpp"

void Firebase::begin(String referenceURL, String apiKey, String authToken) {
    _host = referenceURL;
    _apiKey = apiKey;
    _authToken = authToken;

    if (_host.startsWith("https://")) {
        _host.remove(0, 8);
    }
    if (_host.endsWith("/")) {
        _host.remove(_host.length() - 1);
    }

    _httpsClient.setInsecure();
    _httpsClient.setBufferSizes(_recv, _xmit);
}

void Firebase::signIn(String email, String password) {
    String url = "https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=" + _apiKey;
    if (!_httpsClient.connect("identitytoolkit.googleapis.com", PORT)) {
        _checkSignIn = false;
        return;
    }

    String requestBody = "{\"email\":\"" + email + "\",";
        requestBody += "\"password\":\"" + password + "\",";
        requestBody += "\"returnSecureToken\":true}";

    Serial.println(requestBody);

    _httpsClient.print(String("POST ") + url + " HTTP/1.1\r\n"
        + "Host: identitytoolkit.googleapis.com\r\n"
        + "Content-Type: application/json\r\n"
        + "Content-Length: " + String(requestBody.length()) + "\r\n\r\n"
        + requestBody + "\r\n"
    );

    while (_httpsClient.connected()) {
        String line = _httpsClient.readStringUntil('\n');
        if (line == "\r") break;
    }

    String response;
    while (_httpsClient.available()) {
        response += _httpsClient.readStringUntil('\n');
    }

    int startIndex = response.indexOf("idToken");
    if (startIndex > 0) {
        int endIndex = response.indexOf('"', startIndex + 9);
        String idToken = response.substring(startIndex + 9, endIndex);
        _checkSignIn = true;
    }
    else {
        Serial.println(F("Failed to extract idToken"));
        _checkSignIn = false;
    }
}

void Firebase::connect_to_host() {
    int r = 0;
    while ((!_httpsClient.connect(_host, PORT)) && (r < 30)) {
        delay(100);
        r++;
    }
}

// void Firebase::json(bool json) {
//     _json = json;
// }

int Firebase::set(String path, String msg) {
    if (!_checkSignIn) {
        // Failed to SignIn account
        Serial.println(F("Failed to sign in"));
        return 400;
    }
    
    connect_to_host();
    String jsonObejct = "/" + path + ".json?auth=" + _authToken;

    _httpsClient.print(String("PUT ") + jsonObejct + " HTTP/1.1\r\n" +
        "Host: " + _host + "\r\n" +
        "Connection: close\r\n" +
        "Accept: */*\r\n" +
        "User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n" +
        "Content-Type: application/json;charset=utf-8\r\n" +
        "Content-Length: " + msg.length() + "\r\n" + "\r\n" +
        msg + "\r\n"
    );

    while (_httpsClient.connected()) {
        String line = _httpsClient.readStringUntil('\n');
        if (line == "\r") break;
    }

    while (_httpsClient.available()) {
        String line = _httpsClient.readStringUntil('\n');
        if (line.length() > 0) return 200; // Success
    }

    return 400; // Failed
}

int Firebase::setString(String path, String data) {
    String _data = "\"" + data + "\"";
    return this->set(path, _data);
}

int Firebase::setInt(String path, int data) {
    return this->set(path, String(data));
}

int Firebase::setFloat(String path, float data) {
    return this->set(path, String(data));
}

int Firebase::setBool(String path, bool data) {
    return this->set(path, data ? "true" : "false");
}

int Firebase::push(String path, String msg) {
    if (!_checkSignIn) {
        // Failed to SignIn account
        Serial.println(F("Failed to sign in"));
        return 400;
    }
    
    connect_to_host();
    String jsonObject = "/" + path + ".json?auth=" + _authToken;

    _httpsClient.print(String("POST ") + jsonObject + " HTTP/1.1\r\n" +
        "Host: " + _host + "\r\n" +
        "Connection: close\r\n" +
        "Accept: */*\r\n" +
        "User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n" +
        "Content-Type: application/json;charset=utf-8\r\n" +
        "Content-Length: " + msg.length() + "\r\n" + "\r\n" +
        msg + "\r\n"
    );

    while (_httpsClient.connected()) {
        String line = _httpsClient.readStringUntil('\n');
        if (line == "\r") break;
    }

    while (_httpsClient.available()) {
        String line = _httpsClient.readStringUntil('\n');
        if (line.length() > 0) return 200; // Success
    }

    return 400; // Failed
}

int Firebase::pushString(String path, String data) {
    String _data = "\"" + data +"\"";
    return this->push(path, _data);
}

int Firebase::pushInt(String path, int data) {
    return this->push(path, String(data));
}

int Firebase::pushFloat(String path, float data) {
    return this->push(path, String(data));
}

int Firebase::pushBool(String path, bool data) {
    return this->push(path, data ? "true" : "false");
}

String Firebase::get(String path) {
    if (!_checkSignIn) {
        // Failed to SignIn account
        Serial.println(F("Failed to sign in"));
        _int = 0;
        _float = 0;
        _String = "";
        return _String;
    }
    
    connect_to_host();
    String jsonObject = "/" + path + ".json?auth=" + _authToken;
    
    _httpsClient.print(String("GET ") + jsonObject + " HTTP/1.1\r\n" +
        "Host: " + _host + "\r\n" +
        "Connection: close\r\n\r\n"
    );

    while (_httpsClient.connected()) {
        String line = _httpsClient.readStringUntil('\n');
        if (line == "\r") break;
    }

    String line;
    while (_httpsClient.available()) {
        line = _httpsClient.readStringUntil('\n');
        _int = line.toInt();
        _float = line.toFloat();
        _bool = line.equals("true");
        _String = line.substring(1, line.length() - 1);
        // _String = (_json ? line : line.substring(1, line.length() - 1));
    }

    return line;
}

String Firebase::getString(String path) {
    this->get(path);
    return _String;
}

int Firebase::getInt(String path) {
    this->get(path);
    return _int;
}

float Firebase::getFloat(String path) {
    this->get(path);
    return _float;
}

bool Firebase::getBool(String path) {
    this->get(path);
    return _bool;
}

int Firebase::del(String path) {
    if (!_checkSignIn) {
        // Failed to SignIn account
        Serial.println(F("Failed to sign in"));
        return 400;
    }

    connect_to_host();
    String jsonObject = "/" + path + ".json?auth=" + _authToken;

    _httpsClient.print(String("DELETE ") + jsonObject + " HTTP/1.1\r\n" + 
        "Host: " + _host + "\r\n" +
        "Connection: close\r\n\r\n"
    );

    while (_httpsClient.connected()) {
        String line = _httpsClient.readStringUntil('\n');
        if (line == "\r") break;
    }

    while (_httpsClient.readStringUntil('\n')) {
        String line = _httpsClient.readStringUntil('\n');
        if (line.length() > 0) return 200; // Success
    }

    return 400; // Failed
}