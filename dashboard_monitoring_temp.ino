#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"

// =========================
// KONFIGURASI DHT11
// =========================
#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// =========================
// WIFI
// =========================
const char* ssid = "My Galaxy_4G";
const char* password = "mygalaxy00";

// =========================
// WEB SERVER
// =========================
WebServer server(80);

// =========================
// API SENSOR
// =========================
void handleData() {

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {

    String json = "{";
    json += "\"temperature\":0,";
    json += "\"humidity\":0";
    json += "}";

    server.send(200, "application/json", json);
    return;
  }

  String json = "{";
  json += "\"temperature\":" + String(t, 1) + ",";
  json += "\"humidity\":" + String(h, 1);
  json += "}";

  server.send(200, "application/json", json);
}

// =========================
// HALAMAN WEB
// =========================
void handleRoot() {

  String html = R"rawliteral(

<!DOCTYPE html>
<html lang="en">

<head>

<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">

<title>Smart Temp & Humidity Monitor</title>

<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>

<style>

*{
  margin:0;
  padding:0;
  box-sizing:border-box;
}

body{
  font-family:Arial, sans-serif;
  background:linear-gradient(180deg,#081120,#0f172a);
  color:white;
  overflow-x:hidden;
}

.container{
  width:90%;
  max-width:1200px;
  margin:auto;
  padding:20px;
}

.title{
  text-align:center;
  font-size:38px;
  font-weight:bold;
  margin-top:20px;
  margin-bottom:10px;
  color:#38bdf8;
  text-shadow:0 0 20px rgba(56,189,248,0.7);
}

.subtitle{
  text-align:center;
  opacity:0.7;
  margin-bottom:35px;
}

.cards{
  display:flex;
  flex-wrap:wrap;
  justify-content:center;
  gap:25px;
}

.card{
  background:#1e293b;
  width:320px;
  padding:25px;
  border-radius:25px;
  box-shadow:0 0 25px rgba(56,189,248,0.2);
  transition:0.3s;
}

.card:hover{
  transform:translateY(-5px);
  box-shadow:0 0 30px rgba(56,189,248,0.4);
}

.label{
  font-size:20px;
  opacity:0.8;
  margin-bottom:15px;
}

.value{
  font-size:55px;
  font-weight:bold;
  margin-bottom:15px;
}

.status{
  margin-bottom:15px;
  font-size:16px;
  font-weight:bold;
}

.progress-container{
  width:100%;
  height:22px;
  background:#334155;
  border-radius:30px;
  overflow:hidden;
}

.progress-bar{
  height:100%;
  width:0%;
  border-radius:30px;
  transition:1s;
}

.temp-bar{
  background:linear-gradient(90deg,#38bdf8,#818cf8);
}

.hum-bar{
  background:linear-gradient(90deg,#06b6d4,#3b82f6);
}

.chart-box{
  margin-top:35px;
  background:#1e293b;
  border-radius:30px;
  padding:25px;
  box-shadow:0 0 25px rgba(129,140,248,0.2);
}

.footer{
  text-align:center;
  margin-top:30px;
  margin-bottom:20px;
  opacity:0.6;
}

canvas{
  max-height:400px;
}

</style>

</head>

<body>

<div class="container">

<div class="title">
🌡️ ESP32 Temperature Monitoring
</div>

<div class="cards">

<div class="card">

<div class="label">
Temperature
</div>

<div class="value" id="temp">
-- °C
</div>

<div class="status" id="tempStatus">
Waiting data...
</div>

<div class="progress-container">
<div class="progress-bar temp-bar" id="tempBar"></div>
</div>

</div>

<div class="card">

<div class="label">
Humidity
</div>

<div class="value" id="hum">
-- %
</div>

<div class="status" id="humStatus">
Waiting data...
</div>

<div class="progress-container">
<div class="progress-bar hum-bar" id="humBar"></div>
</div>

</div>

</div>

<div class="chart-box">

<canvas id="tempChart"></canvas>

</div>

<div class="footer">
✨Real-Time Temperature & Humidity Monitoring with ESP32
</div>

</div>

<script>

const tempText = document.getElementById("temp");
const humText = document.getElementById("hum");

const tempBar = document.getElementById("tempBar");
const humBar = document.getElementById("humBar");

const tempStatus = document.getElementById("tempStatus");
const humStatus = document.getElementById("humStatus");

const ctx = document.getElementById("tempChart").getContext("2d");

let labels = [];
let tempData = [];

const chart = new Chart(ctx, {

    type: "line",

    data: {

        labels: labels,

        datasets: [{

            label: "Temperature (°C)",

            data: tempData,

            borderColor: "#38bdf8",

            backgroundColor: "rgba(56,189,248,0.15)",

            borderWidth: 4,

            pointRadius: 5,

            pointHoverRadius: 8,

            tension: 0.4,

            fill: true

        }]
    },

    options: {

        responsive: true,

        animation: {
          duration: 1000
        },

        plugins: {

            legend: {
                labels: {
                    color: "white",
                    font: {
                      size: 14
                    }
                }
            }
        },

        scales: {

            x: {

                ticks: {
                    color: "white"
                },

                grid: {
                    color: "rgba(255,255,255,0.05)"
                }
            },

            y: {

                ticks: {
                    color: "white"
                },

                grid: {
                    color: "rgba(255,255,255,0.05)"
                },

                suggestedMin: 20,
                suggestedMax: 35
            }
        }
    }
});

async function getData(){

    try{

      const response = await fetch("/data");

      const data = await response.json();

      const temp = data.temperature;
      const hum = data.humidity;

      // TEXT
      tempText.innerHTML = temp + " °C";
      humText.innerHTML = hum + " %";

      // PROGRESS BAR
      tempBar.style.width = (temp * 2) + "%";
      humBar.style.width = hum + "%";

      // STATUS TEMPERATURE
      if(temp < 27){
        tempStatus.innerHTML = "Normal";
      }
      else if(temp >= 27 && temp < 32){
        tempStatus.innerHTML = "Warm";
      }
      else{
        tempStatus.innerHTML = "Hot";
      }

      // STATUS HUMIDITY
      if(hum < 40){
        humStatus.innerHTML = "Dry";
      }
      else if(hum >= 40 && hum < 70){
        humStatus.innerHTML = "Comfort";
      }
      else{
        humStatus.innerHTML = "Humid";
      }

      // TIME
      const now = new Date().toLocaleTimeString();

      labels.push(now);
      tempData.push(temp);

      // MAX DATA
      if(labels.length > 15){

        labels.shift();
        tempData.shift();
      }

      // AUTO SCALE BIAR GAK GARIS LURUS MULU 😭
      let minTemp = Math.min(...tempData) - 1;
      let maxTemp = Math.max(...tempData) + 1;

      chart.options.scales.y.min = minTemp;
      chart.options.scales.y.max = maxTemp;

      chart.update();

    }catch(error){

      console.log("Error ambil data");
    }
}

setInterval(getData, 2000);

getData();

</script>

</body>
</html>

)rawliteral";

  server.send(200, "text/html", html);
}

// =========================
// SETUP
// =========================
void setup() {

  Serial.begin(115200);

  dht.begin();

  WiFi.begin(ssid, password);

  Serial.print("Menghubungkan ke WiFi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi Terhubung!");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);

  server.on("/data", handleData);

  server.begin();

  Serial.println("Web server berjalan...");
}

// =========================
// LOOP
// =========================
void loop() {

  server.handleClient();
}