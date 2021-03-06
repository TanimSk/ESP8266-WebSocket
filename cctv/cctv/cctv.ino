#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

Servo servo_x;
Servo servo_y;

StaticJsonDocument<48> doc;

#define ssid "OPTIMUS-Access-Point"
#define password "123opop123"

#define servo_x_pin D7
#define servo_y_pin D5

String str_payload = "";
bool toggle_rotation = false;
unsigned long now_time = 0;

int servo_x_val = 0;
bool flag_x = true;

const char *html_content = "<!DOCTYPE html>\n"
                           "<html lang=\"en\">\n"
                           "\n"
                           "<head>\n"
                           "  <meta charset=\"UTF-8\">\n"
                           "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
                           "  <meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">\n"
                           "  <title>\n"
                           "    Mousebot\n"
                           "  </title>\n"
                           "  <meta name=\"viewport\" content=\"user-scalable=no\">\n"
                           "\n"
                           "  <style>\n"
                           "    .btn{\n"
                           "      height: 3rem;\n"
                           "      width: 7rem;\n"
                           "      background-color: rgb(123, 212, 123);\n"
                           "      border: none;\n"
                           "      outline: none;\n"
                           "      transition: .3s;\n"
                           "      border-radius: 10px;\n"
                           "      color: yellow;\n"
                           "    }\n"
                           "    .btn:hover{\n"
                           "      background-color: rgb(93, 158, 93);\n"
                           "    }\n"
                           "    \n"
                           "  </style>\n"
                           "\n"
                           "</head>\n"
                           "\n"
                           "<body style=\"position: fixed; font-family: 'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif ;\n"
                           "color:rgb(128, 128, 128);\n"
                           "font-size: xx-large;\">\n"
                           "  \n"
                           "  <div style=\"text-align: center;\">\n"
                           "    <button class=\"btn\" onclick=\"toggle_rotation()\">\n"
                           "      <b>Rotation</b>\n"
                           "    </button>\n"
                           "  </div>\n"
                           "\n"
                           "  <canvas id=\"canvas\" name=\"game\"></canvas> \n"
                           "\n"
                           "  <script>\n"
                           "    let Socket = new WebSocket('ws://' + window.location.hostname + ':81/');\n"
                           "    function send(x, y) {\n"
                           "      data = {\n"
                           "        'x': x,\n"
                           "        'y': y\n"
                           "      }\n"
                           "      Socket.send(JSON.stringify(data));\n"
                           "    }\n"
                           "  </script>\n"
                           "  <script>\n"
                           "    var canvas, ctx;\n"
                           "    window.addEventListener('load', () => {\n"
                           "      canvas = document.getElementById('canvas');\n"
                           "      ctx = canvas.getContext('2d');\n"
                           "      resize();\n"
                           "\n"
                           "      document.addEventListener('mousedown', startDrawing);\n"
                           "      document.addEventListener('mouseup', stopDrawing);\n"
                           "      document.addEventListener('mousemove', Draw);\n"
                           "\n"
                           "      document.addEventListener('touchstart', startDrawing);\n"
                           "      document.addEventListener('touchend', stopDrawing);\n"
                           "      document.addEventListener('touchcancel', stopDrawing);\n"
                           "      document.addEventListener('touchmove', Draw);\n"
                           "      window.addEventListener('resize', resize);\n"
                           "    });\n"
                           "\n"
                           "    var width, height, radius, x_orig, y_orig;\n"
                           "\n"
                           "    function resize() {\n"
                           "      width = window.innerWidth;\n"
                           "      radius = 200;\n"
                           "      height = radius * 6.5;\n"
                           "      ctx.canvas.width = width;\n"
                           "      ctx.canvas.height = height;\n"
                           "      background();\n"
                           "      joystick(width / 2, height / 3);\n"
                           "    }\n"
                           "\n"
                           "    let btn_toggle = true;\n"
                           "\n"
                           "    function toggle_rotation(){\n"
                           "      var xmlHttp = new XMLHttpRequest();\n"
                           "      xmlHttp.open(\"GET\", \"/rotate\", true);\n"
                           "      xmlHttp.send(null);\n"
                           "\n"
                           "      btn_toggle = !btn_toggle;\n"
                           "    }\n"
                           "\n"
                           "    function background() {\n"
                           "      x_orig = width / 2;\n"
                           "      y_orig = height / 3;\n"
                           "\n"
                           "      ctx.beginPath();\n"
                           "      ctx.arc(x_orig, y_orig, radius + 20, 0, Math.PI * 2, true);\n"
                           "      ctx.fillStyle = '#ECE5E5';\n"
                           "      ctx.fill();\n"
                           "    }\n"
                           "\n"
                           "    function joystick(width, height) {\n"
                           "      ctx.beginPath();\n"
                           "      ctx.arc(width, height, radius, 0, Math.PI * 2, true);\n"
                           "      ctx.fillStyle = '#F08080';\n"
                           "      ctx.fill();\n"
                           "      ctx.strokeStyle = '#F6ABAB';\n"
                           "      ctx.lineWidth = 8;\n"
                           "      ctx.stroke();\n"
                           "    }\n"
                           "\n"
                           "    let coord = {\n"
                           "      x: 0,\n"
                           "      y: 0\n"
                           "    };\n"
                           "    let paint = false;\n"
                           "\n"
                           "    function getPosition(event) {\n"
                           "      var mouse_x = event.clientX || event.touches[0].clientX;\n"
                           "      var mouse_y = event.clientY || event.touches[0].clientY;\n"
                           "      coord.x = mouse_x - canvas.offsetLeft;\n"
                           "      coord.y = mouse_y - canvas.offsetTop;\n"
                           "    }\n"
                           "\n"
                           "    function is_it_in_the_circle() {\n"
                           "      var current_radius = Math.sqrt(Math.pow(coord.x - x_orig, 2) + Math.pow(coord.y - y_orig, 2));\n"
                           "      if (radius >= current_radius) return true\n"
                           "      else return false\n"
                           "    }\n"
                           "\n"
                           "    function startDrawing(event) {\n"
                           "      paint = true;\n"
                           "      getPosition(event);\n"
                           "      if (is_it_in_the_circle()) {\n"
                           "        ctx.clearRect(0, 0, canvas.width, canvas.height);\n"
                           "        background();\n"
                           "        joystick(coord.x, coord.y);\n"
                           "        Draw();\n"
                           "      }\n"
                           "    }\n"
                           "\n"
                           "    function stopDrawing() {\n"
                           "      paint = false;\n"
                           "    }\n"
                           "\n"
                           "    function Draw(event) {\n"
                           "      if (paint) {\n"
                           "        ctx.clearRect(0, 0, canvas.width, canvas.height);\n"
                           "        background();\n"
                           "        var angle_in_degrees, x, y, speed;\n"
                           "        var angle = Math.atan2((coord.y - y_orig), (coord.x - x_orig));\n"
                           "        if (Math.sign(angle) == -1) {\n"
                           "          angle_in_degrees = Math.round(-angle * 180 / Math.PI);\n"
                           "        } else {\n"
                           "          angle_in_degrees = Math.round(360 - angle * 180 / Math.PI);\n"
                           "        }\n"
                           "        if (is_it_in_the_circle()) {\n"
                           "          joystick(coord.x, coord.y);\n"
                           "          x = coord.x;\n"
                           "          y = coord.y;\n"
                           "        } else {\n"
                           "          x = radius * Math.cos(angle) + x_orig;\n"
                           "          y = radius * Math.sin(angle) + y_orig;\n"
                           "          joystick(x, y);\n"
                           "        }\n"
                           "        getPosition(event);\n"
                           "        var speed = Math.round(100 * Math.sqrt(Math.pow(x - x_orig, 2) + Math.pow(y - y_orig, 2)) / radius);\n"
                           "        var y_relative = Math.round(x - x_orig);\n"
                           "        var x_relative = Math.round(y - y_orig);\n"
                           "        if(x_relative % 2 == 0 && y_relative % 2 == 0){\n"
                           "          send(500 + (y_relative+200)*5, 500 + (x_relative+200)*5);\n"
                           "        }\n"
                           "      }\n"
                           "    }\n"
                           "  </script>\n"
                           "</body>\n"
                           "</html><br>\n";

void main_page() {
  server.send(200, "text/html", html_content);
}

void auto_rotate() {
  toggle_rotation = ! toggle_rotation;
  server.send(200, "text/plain", "");
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    str_payload = String((char *)payload);
    deserializeJson(doc, str_payload);
    if(!toggle_rotation) servo_x.writeMicroseconds(doc["x"]);
    servo_y.writeMicroseconds(doc["y"]);
  }
}


void setup() {

  servo_x.attach(servo_x_pin);
  servo_y.attach(servo_y_pin);

  servo_x.write(90);
  servo_y.write(90);

  delay(300);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", main_page);
  server.on("/rotate", auto_rotate);

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  Serial.println("Server listening");
}


void rotate_servo_x() {
  if (servo_x_val >= 180) {
    servo_x_val = 180;
    flag_x = false;
  }
  else if (servo_x_val <= 0) {
    servo_x_val = 0;
    flag_x = true;
  }
  if (flag_x) {
    servo_x_val += 5;
  }
  else {
    servo_x_val -= 5;
  }
  servo_x.write(servo_x_val);
}


void loop() {
  webSocket.loop();
  server.handleClient();

  if (toggle_rotation) {
    if ((millis() - now_time) > 350) {
      rotate_servo_x();
      now_time = millis();
    }
  }

}
