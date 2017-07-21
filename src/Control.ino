/*
 * Función para obtener el sensor activo más a la derecha
 */
int sensorDerecha (bool lectura_CNYS_para_PD[6])
{
 if (lectura_CNYS_para_PD[5] == 1) return(6); // Sensor más a la derecha
 if (lectura_CNYS_para_PD[4] == 1) return(5);
 if (lectura_CNYS_para_PD[3] == 1) return(4);
 if (lectura_CNYS_para_PD[2] == 1) return(3);
 if (lectura_CNYS_para_PD[1] == 1) return(2);
 if (lectura_CNYS_para_PD[0] == 1) return(1); // Sensor más a la izquierda
 return(0); // No se detecta la línea
}

/*
 * Función para obtener el sensor activo más a la izquierda
 */
int sensorIzquierda (bool lectura_CNYS_para_PD[6])
{
 if (lectura_CNYS_para_PD[0] == 1) return(1); // Sensor más a la izquierda
 if (lectura_CNYS_para_PD[1] == 1) return(2);
 if (lectura_CNYS_para_PD[2] == 1) return(3);
 if (lectura_CNYS_para_PD[3] == 1) return(4);
 if (lectura_CNYS_para_PD[4] == 1) return(5);
 if (lectura_CNYS_para_PD[5] == 1) return(6); // Sensor más a la derecha
 return(0); // No se detecta la línea
}

/*
 * Función para detectar si se está leyendo negro en todos los sensores, lo que
 * podría indicar una marca de frenada en la pista
 */
void marcaFrenada ()
{
  if (lectura_CNYS[0] == 1
      && lectura_CNYS[1] == 1
      && lectura_CNYS[2] == 1
      && lectura_CNYS[3] == 1
      && lectura_CNYS[4] == 1
      && lectura_CNYS[5] == 1)
  {
    parar_velocista_negro++;
  }
}

/*
 * Funciones para el cálculo de la dirección
 */
void calculoDireccion () // Para dirección normal
{
  direccion_actual = sensorDerecha(lectura_CNYS) + sensorIzquierda(lectura_CNYS);
}

/*
 * Función de control PD adaptativo
 */
 void controlPDadaptativo ()
 {
   if (direccion == 2 || direccion == 12)
   {
     VELOCIDAD_BASE = 220;
     KP = 22;
     KD = 260;
   }
   else if (direccion == 3 || direccion == 11)
   {
     VELOCIDAD_BASE = 220;
     KP = 22;
     KD = 260;
   }
   else if (direccion == 5 || direccion == 9)
   {
     VELOCIDAD_BASE = 255;
     KP = 26;
     KD = 300;
   }
   else if (direccion == 7)
   {
     VELOCIDAD_BASE = 255;
     KP = 26;
     KD = 300;
   }
 }

/*
 * Función para el control PD
 */
int controlPD ()
{
  int control; // La variable control es la salida del PID

  direccion = direccion_actual;

  // Si perdemos la línea seguimos en la última dirección detectada
  if (direccion == 0) {direccion = direccion_anterior; parar_velocista_blanco++;}
  else                {direccion_anterior = direccion; parar_velocista_blanco = 0;}

  //controlPDadaptativo ();

  error = direccion - REFERENCIA_DIRECCION; // Error respecto a la dirección que se debe tomar

  control = error * KP + (error - error_anterior) * KD; // Control PD

  error_anterior = error; // Parte derivativa

  return(control);
}

/*
 * Función para actuar sobre los motores con la salida del PID
 */
void actuacionMotores (int control_pwm)
{
  int md_pwm; // Señal PWM del motor derecho
  int mi_pwm; // Señal PWM del motor izquierda

  md_pwm = VELOCIDAD_BASE - control_pwm; // Valor del motor derecho
  mi_pwm = VELOCIDAD_BASE + control_pwm; // Valor del motor izquierdo

  controlMotores (mi_pwm, md_pwm); // Actuación de los motores
}

/*
  Funciones de los encoders
  */
void encoderIzquierdo ()
{

}

void encoderDerecho ()
{

}
