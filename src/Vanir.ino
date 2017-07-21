/***********************************************************/
/* Velocista Vanir                                         */
/*                                                         */
/* Proyecto de robot rastreador basado en el XXXX de AMUVa */
/*                                                         */
/* Creador: Raúl Hurtado Gavilán                           */
/* 2017-04-01                                              */
/***********************************************************/

/*
 * DECLACIÓN DE PINES Y COMPONENTES
 */
// LEDS y BOTONES
#define LED_1 10 // Led azul
#define LED_2 11 // Led rojo

#define BOTON_1 12 // Botón 1
#define BOTON_2 13 // Botón 2

// CNYS
#define CNY_1 A7 // Izquierdo
#define CNY_2 A6
#define CNY_3 A3
#define CNY_4 A2
#define CNY_5 A1
#define CNY_6 A0 // Derecho

// I2C
#define SDA A4 // Central
#define SLC A5 // Central

// ENCODERS
#define ENCODER_IZQUIERDO 2
#define ENCONDER_DERECHO 3

// MOTORES
#define PWM_MOTOR_IZQUIERDO 6 // Motor izquierdo 1
#define ENTRADA_A_MOTOR_IZQUIERDO 5
#define ENTRADA_B_MOTOR_IZQUIERDO 4

#define PWM_MOTOR_DERECHO 9 // Motor derecho 2
#define ENTRADA_A_MOTOR_DERECHO 8
#define ENTRADA_B_MOTOR_DERECHO 7

/*
 * LECTURA SENSORES
 */
bool lectura_CNYS[6]; // Lectura de los sensores para el controlador PD

/*
 *CONTROL PID
 */
float error = 0; // La variable error es la parte proporcional
float error_acumulado = 0; // La variable error_acumulado es la parte integral
float error_anterior = 0; // La variable error_anterior es la parte derivativa

int direccion; // Dirección que se va a tomar en el PD
int direccion_actual; // Dirección actual de la lectura de los sensores activos
int direccion_anterior; // Dirección anterior de la lectura de los sensores activos
int REFERENCIA_DIRECCION = 7; // Dirección dde referencia

// Para 100: 20 y 50
// Para 160: 10,50??
// Para 180: 12,200
// Para 200: 18,225
// Para 220: 22,260
// PAra 255: 30,260

int KP = 30; // Constante proporcional #40
int KD = 260; // Constante derivativa #2500

int VELOCIDAD_BASE = 255; // Velocidad base

int control_pwm; // Señal del PID para los motores
int md_pwm; // Señal para motor derecho
int mi_pwm; // Señal par motor izquierdo

int parar_velocista_blanco = 0; // Variable para parar el velocista si lleva mucho tiempo sin leer nada
int parar_velocista_negro = 0; // Variable para parar el velocista si lleva mucho tiempo en una marca de parada

int MARCA_PARADA_BLANCO = 1000; // Variable para el tiempo de parada en color blanco de la pista
int MARCA_PARADA_NEGRO = 150; // Variable para el tiempo de parada en color negro de la pista

// Variables para el tiempo del control PD
unsigned long tiempo_comienzo = 0;
unsigned long tiempo_fin = 0;
int tiempo_PD = 3;

/*
 * OTRAS VARIABLES
 */
int RETARDO_REGLAMENTARIO = 500;
int REFERENCIA_COLOR = 180;
bool pausa = 0; // Variable para definir el estado de pausa del rastreador

/*
 * ESTADOS DE LA MÁQUINA DE ESTADOS FINITOS (FSM)
 */
#define REPOSO 1
#define CALIBRACION 2
#define SIGUELINEAS 3

byte estado_finito = REPOSO;


/*
 * FUNCIÓN SETUP
 */
void setup()
{
  // PINES DE ENTRADA
  pinMode (BOTON_1, INPUT);
  pinMode (BOTON_2, INPUT);

  pinMode (CNY_1, INPUT);
  pinMode (CNY_2, INPUT);
  pinMode (CNY_3, INPUT);
  pinMode (CNY_4, INPUT);
  pinMode (CNY_5, INPUT);
  pinMode (CNY_6, INPUT);

  // PINES DE SALIDA
  pinMode (LED_1, OUTPUT); // LED azul
  pinMode (LED_2, OUTPUT); // LED rojo

  pinMode (PWM_MOTOR_IZQUIERDO,OUTPUT); // Motor izquierdo 1
  pinMode (ENTRADA_A_MOTOR_IZQUIERDO,OUTPUT);
  pinMode (ENTRADA_B_MOTOR_IZQUIERDO,OUTPUT);

  pinMode (PWM_MOTOR_DERECHO,OUTPUT); // Motor derecho 2
  pinMode (ENTRADA_A_MOTOR_DERECHO,OUTPUT);
  pinMode (ENTRADA_B_MOTOR_DERECHO,OUTPUT);

  // INTERRUPCIONES DE ENCODERS
  attachInterrupt(digitalPinToInterrupt(2),encoderIzquierdo, RISING);
  attachInterrupt(digitalPinToInterrupt(3),encoderDerecho, RISING);

  //configurarBluethoot(); // Función para configurar el Bluethoot

  Serial.begin (9600); // Inicia comunicaciones serie a 9600 bps
}

/*
 * FUNCIÓN LOOP //
 */
void loop()
{
  // Máquina de estados finitos
  switch (estado_finito)
  {
    /*
     * 1: Estado de reposo del rastreador
     */
    case REPOSO:

      controlMotores (0, 0); // Motores parados
      pausa = 0; // Para que al pasar al estado SIGUELÍNEAS comience la cuenta atrás

      // Se resetean las variables de salida de pista
      parar_velocista_blanco = 0;
      parar_velocista_negro = 0;

      digitalWrite (LED_1, LOW);
      digitalWrite (LED_2, HIGH); // Señal de estado REPOSO

      // Elección de estado SIGUELINEAS
      if (digitalRead (BOTON_2) == HIGH)
      {
        delay (200);
        estado_finito = SIGUELINEAS;
      }
      // Elección de estado CALIBRACION
      /*if (digitalRead (BOTON_2) == HIGH)
      {
        delay (200);
        estado_finito = CALIBRACION;
      }*/
      break;

    /*
     * 2: Estado de calibración de los sensores
     */
    case CALIBRACION:

      digitalWrite (LED_1, HIGH); // Señal de estado CALIBRACION
      digitalWrite (LED_2, LOW);

      // Activado de la calibración
      if (digitalRead (BOTON_2) == HIGH)
      {
        delay (200);
        calibracionCNYS ();
      }
      // Elección de estado SIGUELINEAS
      else if (digitalRead (BOTON_1) == HIGH)
      {
        delay (200);
        estado_finito = SIGUELINEAS;
      }

      break;

    /*
     * 3: Estado normal de siguimiento de línea
     */
    case SIGUELINEAS:

      // Si está a 0, realiza la cuenta atrás, si está a 1 la obvia
      if (pausa == 0)
      {
        pausa = 1;
        digitalWrite (LED_1, LOW);
        digitalWrite (LED_2, HIGH); // Señal de estado SIGUELINEAS

        for (byte i = 0; i < 5; i++)
        {
          digitalWrite (LED_1, HIGH);
          delay (RETARDO_REGLAMENTARIO);
          digitalWrite (LED_1, LOW);
          delay (RETARDO_REGLAMENTARIO   );
        }
      }

      if (parar_velocista_blanco > MARCA_PARADA_BLANCO || parar_velocista_negro > MARCA_PARADA_NEGRO)
      {
        estado_finito = REPOSO;
      }

      lecturaCnys (); // Lectura de los sensores

      marcaFrenada ();

      tiempo_comienzo = millis();

      if (tiempo_comienzo - tiempo_fin >= tiempo_PD)
      {
        //Serial.println(millis());

        calculoDireccion ();
        control_pwm = controlPD (); // Cálculo del PID
        actuacionMotores (control_pwm); // Actuación sobre los motores

        tiempo_fin = millis();
      }

      // Elección de estado REPOSO
      if (digitalRead (BOTON_2) == HIGH)
      {
        delay (500);
        estado_finito = REPOSO;
      }

      break;
  }

  //telemetria (); // Activamos la telemetría
}
