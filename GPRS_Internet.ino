#include <SoftwareSerial.h>
SoftwareSerial SIM900(7, 8); // Configura el puerto serial para el SIM900. Para el Arduino MEGA utilizar pines 10 y 11

int respuesta;
char aux_str[50];

//Contenido de la dirección Http  
char direccion[] = "GET /prueba-http/ HTTP/1.1\r\nHost: www.prometec.net\r\nConnection: close\r\n\r\n";

void setup()
{
  SIM900.begin(19200); //Configura velocidad del puerto serie para el SIM900
  Serial.begin(19200); //Configura velocidad del puerto serie del Arduino
  delay(1000);
  Serial.println("Iniciando...");
  power_on();
  iniciar();
}

void loop()
{
  if (Serial.available())
    switch (Serial.read())
    {
      case 'h':
        PeticionHttp();
        break;
      case 'q':
        // Closes the socket
        enviarAT("AT+CIPCLOSE", "CLOSE OK", 10000); //Cerramos la conexion
        enviarAT("AT+CIPSHUT", "OK", 10000); //Cierra el contexto PDP del GPRS
        delay(10000);
        break;
    }
  if (SIM900.available()) {
    Serial.write(SIM900.read());
  }
}


int enviarAT(String ATcommand, char* resp_correcta, unsigned int tiempo)
{

  int x = 0;
  bool correcto = 0;
  char respuesta[100];
  unsigned long anterior;

  memset(respuesta, '\0', 100); // Inicializa el string
  delay(100);
  while ( SIM900.available() > 0) SIM900.read(); // Limpia el buffer de entrada
  SIM900.println(ATcommand); // Envia el comando AT
  x = 0;
  anterior = millis();
  // Espera una respuesta
  do {
    // si hay datos el buffer de entrada del UART lee y comprueba la respuesta
    if (SIM900.available() != 0)
    {
        respuesta[x] = SIM900.read();
        x++;
      // Comprueba si la respuesta es correcta
      if (strstr(respuesta, resp_correcta) != NULL)
      {
        correcto = 1;
      }
    }
  }
  // Espera hasta tener una respuesta
  while ((correcto == 0) && ((millis() - anterior) < tiempo));
  Serial.println(respuesta);

  return correcto;
}

void power_on()
{
  int respuesta = 0;

  // Comprueba que el modulo SIM900 esta arrancado
  if (enviarAT("AT", "OK", 2000) == 0)
  {
    Serial.println("Encendiendo el GPRS...");

    pinMode(9, OUTPUT);
    digitalWrite(9, HIGH);
    delay(1000);
    digitalWrite(9, LOW);
    delay(1000);

    // Espera la respuesta del modulo SIM900
    while (respuesta == 0) {
      // Envia un comando AT cada 2 segundos y espera la respuesta
      respuesta = enviarAT("AT", "OK", 2000);
      SIM900.println(respuesta);
    }
  }
}

void power_off()
{
  digitalWrite(9, HIGH);
  delay(1000);
  digitalWrite(9, LOW);
  delay(1000);
}

void reiniciar()
{
  Serial.println("Reiniciando...");
  power_off();
  delay (5000);
  power_on();
}

void iniciar()
{
  enviarAT("AT+CPIN=\"1867\"", "OK", 1000); //Introducimos el PIN de la SIM
  Serial.println("Conectando a la red...");
  delay (5000);

  //Espera hasta estar conectado a la red movil
  while ( enviarAT("AT+CREG?", "+CREG: 0,1", 1000) == 0 )
  {
  }

  Serial.println("Conectado a la red.");
  enviarAT("AT+CGATT=1\r", "OK", 1000); //Iniciamos la conexión GPRS
  enviarAT("AT+CSTT=\"ac.vodafone.es\",\"vodafone\",\"vodafone\"", "OK", 3000); //Definimos el APN, usuario y clave a utilizar
  enviarAT("AT+CIICR", "OK", 3000); //Activamos el perfil de datos inalámbrico
  enviarAT("AT+CIFSR", "", 3000); //Activamos el perfil de datos inalámbrico
}

void PeticionHttp()
{
  if (enviarAT("AT+CREG?", "+CREG: 0,1", 1000) == 1) //Comprueba la conexion a la red
  {
    enviarAT("AT+CIPSTART=\"TCP\",\"www.prometec.net\",\"80\"", "CONNECT OK", 5000); //Inicia una conexión TCP
    // Envíamos datos a través del TCP
    sprintf(aux_str, "AT+CIPSEND=%d", strlen(direccion));
    if (enviarAT(aux_str, ">", 10000) == 1)
    {
      enviarAT(direccion, "OK", 10000);
    }
  }
  else
  {
    reiniciar();
    iniciar();
  }
}

