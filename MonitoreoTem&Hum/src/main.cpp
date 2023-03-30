#include <Adafruit_Sensor.h>

#include <DHT_U.h>
#include "ESP8266WiFi.h"

#include <ESP8266HTTPClient.h>

#define DEBUG true

/*
Nota importante: si usas el pin D8 (como lo recomiendo)
  recuerda desconectar el lector del mismo cada vez que reinicies
  o quieras subir el código, pues el mismo "interfiere" con el
  monitor serial
*/

#define DHTPIN D8     // A cuál pin está conectado el lector
#define DHTTYPE DHT22 // Puede ser DHT11 también uint8_t DHTPIN = D8;
DHT dht(DHTPIN, DHTTYPE);

float humedad, temperaturaEnGradosCelsius = 0;
int sensor = 1;
int ultimaVezLeido = 0;
long milisegundosDeEsperaParaLeer = 10000; // 2 segundos

const char *ssid = "Estrella";
const char *password = "1005966818";

void setup()
{

  Serial.begin(9600);
  Serial.setTimeout(100);

  Serial.printf("Conectando a %s", ssid);

  pinMode(D2, OUTPUT); // Establecer D2 como salida
  pinMode(D5, OUTPUT); // Establecer D2 como salida
  pinMode(D6, OUTPUT); // Establecer D2 como salida
  pinMode(D7, OUTPUT); // Establecer D2 como salida
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(D2, HIGH); // Encender el LED si la conexión se perdió
    Serial.println("Esperar un momento mientras se conecta al Wifi...");
    Serial.println("Conectando al WiFi...");
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
      digitalWrite(D2, !digitalRead(D2)); // Cambiar el estado del LED
    }
    Serial.println("");
    Serial.println("WiFi conectado");
  }
  else
  {
    digitalWrite(D2, HIGH); // Encender el LED si está conectado
    Serial.println(WiFi.localIP());
  }

  pinMode(DHTPIN, INPUT);

  pinMode(DHTPIN, INPUT);

  // Muy importante la siguiente línea, pues configura el pin del sensor como INPUT_PULLUP
  dht.begin();

  while (!Serial)
  {
  } // Esperar que haya conexión serial

  Serial.println("Bienvenido");
  Serial.println("Iniciando Monitoreo");
}

void loop()
{

  // comprobamos si la conexion se ha perdido, si es asi se apaga el led, e intenta reconectarse.

  if (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(D2, HIGH); // Encender el LED si la conexión se perdió
    Serial.println("Conexión perdida...");
    Serial.println("Reconectando al WiFi...");
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
      digitalWrite(D2, !digitalRead(D2)); // Cambiar el estado del LED
    }
    Serial.println("");
    Serial.println("WiFi conectado");
  }
  else
  {
    digitalWrite(D2, HIGH); // Encender el LED si está conectado
  }

  // Debemos leer cada 10 segundos
  if (ultimaVezLeido > milisegundosDeEsperaParaLeer)
  {
    humedad = dht.readHumidity();
    temperaturaEnGradosCelsius = dht.readTemperature();
    // En ocasiones puede devolver datos erróneos; por eso lo comprobamos
    while (isnan(temperaturaEnGradosCelsius) || isnan(humedad))
    {
      delay(500);
      Serial.println("Error leyendo valores");
      digitalWrite(D5, HIGH); // Encender LED rojo
      delay(500);             // Encender durante 500 ms
      digitalWrite(D5, LOW);  // Apagar LED rojo
      delay(500);             // Esperar durante 500 ms

      ultimaVezLeido = 0;
      return;
    }
    // En caso de que todo esté correcto, imprimimos los valores
    Serial.print("Humedad: ");
    Serial.print(humedad);
    Serial.print(" %\t");
    Serial.print("Temperatura: ");
    Serial.print(temperaturaEnGradosCelsius);
    Serial.println(" *C");

    String url = "http://192.168.0.27/sensor/save.php?Id_sensor=";
    url += sensor;
    url += "&Temperatura=";
    url += temperaturaEnGradosCelsius;
    url += "&Humedad=";
    url += humedad;

    char OutputJSONMessageBuff[64];
    WiFiClient client;
    HTTPClient http;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(OutputJSONMessageBuff);
    while (httpCode != 200)
    {
      Serial.println("Error: no se pudo establecer la conexión. Intentando de nuevo en 5 segundos...");
      digitalWrite(D7, HIGH); // Encender LED rojo
      delay(500);             // Encender durante 500 ms
      digitalWrite(D7, LOW);  // Apagar LED rojo
      delay(500);             // Esperar durante 500 ms
      httpCode = http.POST(OutputJSONMessageBuff);
    }
    String payload = http.getString();
    Serial.println(httpCode);
    Serial.println(payload);

    if (httpCode == 200)
    {
      digitalWrite(D6, HIGH); // Encender LED verde
      delay(1000);
      digitalWrite(D6, LOW); // Apagar LED verde
    }
    else
    {
      Serial.println("Error al enviar los datos.");
      digitalWrite(D5, HIGH); // Encender LED rojo
      delay(1000);
      digitalWrite(D5, LOW); // Apagar LED rojo
    }

    http.end();
    ultimaVezLeido = 0;
  }
  // Aquí podemos hacer otras cosas...
  // Esperamos 100 milisegundos y también los aumentamos al contador, de este
  // modo evitamos un delay de 2000 milisegundos y podemos hacer otras cosas
  // por aquí
  delay(100);
  ultimaVezLeido += 100;
}
