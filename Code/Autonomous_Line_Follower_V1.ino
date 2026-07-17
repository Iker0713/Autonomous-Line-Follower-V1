/*****************************************************
 *        ROBOT SEGUIDOR DE LÍNEA V2.0
 *          Versión Optimizada
 *****************************************************/


//=================== PINES ===================

// Sensores
const byte sensIzqExt = 2;
const byte sensIzqInt = 3;
const byte sensCentro = 4;
const byte sensDerInt = 5;
const byte sensDerExt = 6;

// Motor trasero
const byte motorTrasAtras    = 7;
const byte motorTrasAdelante = 8;
const byte motorDelDer       = 9;
const byte motorDelIzq       = 10;
const byte velTrasera        = 11;


//=================== CONFIGURACIÓN ===================

const byte VELOCIDAD_RECTA = 210;
const byte VELOCIDAD_CURVA = 150;

const byte FUERZA_KICK = 255;
const byte FUERZA_MANTENIMIENTO = 190;

const unsigned long TIEMPO_KICK = 70;
const unsigned long TIEMPO_TELEMETRIA = 250;


//=================== ENUM ===================

enum Modo
{
    MANUAL,
    AUTONOMO
};

enum Direccion
{
    RECTO,
    IZQUIERDA,
    DERECHA
};


//=================== VARIABLES ===================

Modo modoActual = MANUAL;
Direccion direccionActual = RECTO;

char comando = ' ';

bool avanzarManual = false;

Direccion ultimaDireccion = RECTO;

unsigned long tiempoLineaPerdida = 0;
bool lineaPerdida = false;

//Sensores

byte s1;
byte s2;
byte s3;
byte s4;
byte s5;


//Timers

unsigned long tiempoTelemetria = 0;
unsigned long tiempoKick = 0;


//Kickstart

bool kickActivo = false;


//=================== SETUP ===================

void setup()
{
    Serial.begin(9600);

    Serial.println();
    Serial.println("===== ROBOT V2.0 =====");
    Serial.println();

    pinMode(motorTrasAdelante, OUTPUT);
    pinMode(motorTrasAtras, OUTPUT);
    pinMode(velTrasera, OUTPUT);
    pinMode(motorDelIzq, OUTPUT);
    pinMode(motorDelDer, OUTPUT);

    pinMode(sensIzqExt, INPUT);
    pinMode(sensIzqInt, INPUT);
    pinMode(sensCentro, INPUT);
    pinMode(sensDerInt, INPUT);
    pinMode(sensDerExt, INPUT);

    detenerMotor();
    ponerRecto();
}



//=================== LOOP ===================

void loop()
{
    leerBluetooth();

    leerSensores();

    imprimirTelemetria();

    actualizarKickstart();

    if(modoActual == MANUAL)
    {
        ejecutarModoManual();
    }
    else
    {
        ejecutarModoAutonomo();
    }
}



//=================== BLUETOOTH ===================

void leerBluetooth()
{
    while(Serial.available())
    {
        comando = Serial.read();

        switch(comando)
        {
            case 'A':

                modoActual = AUTONOMO;
                avanzarManual = false;

                detenerMotor();
                ponerRecto();

                break;


            case 'M':

                modoActual = MANUAL;
                avanzarManual = false;

                detenerMotor();
                ponerRecto();

                break;
        }
    }
}



//=================== SENSORES ===================

void leerSensores()
{
    s1 = digitalRead(sensIzqExt);
    s2 = digitalRead(sensIzqInt);
    s3 = digitalRead(sensCentro);
    s4 = digitalRead(sensDerInt);
    s5 = digitalRead(sensDerExt);
}



//=================== TELEMETRÍA ===================

void imprimirTelemetria()
{
    if(millis() - tiempoTelemetria < TIEMPO_TELEMETRIA)
        return;

    tiempoTelemetria = millis();

    if(modoActual == MANUAL)
        Serial.print("MANUAL  ");
    else
        Serial.print("AUTO    ");

    Serial.print(" | ");

    Serial.print(s1);
    Serial.print(" ");

    Serial.print(s2);
    Serial.print(" ");

    Serial.print(s3);
    Serial.print(" ");

    Serial.print(s4);
    Serial.print(" ");

    Serial.print(s5);

    Serial.print(" | ");

    switch(direccionActual)
    {
        case RECTO:
            Serial.println("RECTO");
            break;

        case IZQUIERDA:
            Serial.println("IZQUIERDA");
            break;

        case DERECHA:
            Serial.println("DERECHA");
            break;
    }
}



//=================== PROTOTIPOS ===================

//==================================================
//            MODO MANUAL
//==================================================

void ejecutarModoManual()
{
    switch(comando)
    {
        case 'w':
            avanzarManual = true;
            break;

        case 's':
            avanzarManual = false;
            detenerMotor();
            break;

        case 'a':
            girarIzquierda();
            break;

        case 'd':
            girarDerecha();
            break;

        case 'c':
            ponerRecto();
            break;
    }

    if(avanzarManual)
    {
        analogWrite(velTrasera, VELOCIDAD_RECTA);
        avanzarMotor();
    }
}


//==================================================
//          CONTROL PROPORCIONAL
//==================================================

int calcularError()
{
    int suma = 0;
    int activos = 0;

    if(s1 == LOW){ suma += -2; activos++; }
    if(s2 == LOW){ suma += -1; activos++; }
    if(s3 == LOW){ suma += 0; activos++; }
    if(s4 == LOW){ suma += 1; activos++; }
    if(s5 == LOW){ suma += 2; activos++; }

    if(activos == 0)
        return 99;      // Línea perdida

    return suma / activos;
}


//==================================================
//         MODO AUTÓNOMO
//==================================================

void ejecutarModoAutonomo()
{
    int error = calcularError();

    if(error==99)
{
    if(!lineaPerdida)
    {
        tiempoLineaPerdida = millis();
        lineaPerdida = true;
    }

    analogWrite(velTrasera,110);
    avanzarMotor();

    if(ultimaDireccion==IZQUIERDA)
        girarIzquierda();
    else
        girarDerecha();

    return;
}

lineaPerdida=false;

    avanzarMotor();

    int velocidad = map(abs(error), 0, 2, VELOCIDAD_RECTA, 80);
    analogWrite(velTrasera, velocidad);

    switch(error)
{
    case -2:
    case -1:
        girarIzquierda();
        break;

    case 0:
        ponerRecto();
        break;

    case 1:
    case 2:
        girarDerecha();
        break;
}
}



//==================================================
//        MOTOR TRASERO
//==================================================

void avanzarMotor()
{
    digitalWrite(motorTrasAdelante, HIGH);
    digitalWrite(motorTrasAtras, LOW);
}

void detenerMotor()
{
    digitalWrite(motorTrasAdelante, LOW);
    digitalWrite(motorTrasAtras, LOW);
}



//==================================================
//        DIRECCIÓN
//==================================================

void ponerRecto()
{
    analogWrite(motorDelIzq,15);
    analogWrite(motorDelDer,15);

    direccionActual = RECTO;
}



//==================================================
//          GIRO IZQUIERDA
//==================================================

void girarIzquierda()
{
    if(direccionActual != IZQUIERDA && !kickActivo)
    {
        analogWrite(motorDelIzq,FUERZA_KICK);

        tiempoKick = millis();
        kickActivo = true;

        direccionActual = IZQUIERDA;
        return;
    }

    analogWrite(motorDelIzq,FUERZA_MANTENIMIENTO);
    analogWrite(motorDelDer,0);

    direccionActual = IZQUIERDA;
    ultimaDireccion = IZQUIERDA;
}



//==================================================
//          GIRO DERECHA
//==================================================

void girarDerecha()
{
    if(direccionActual != DERECHA && !kickActivo)
    {
        analogWrite(motorDelDer,FUERZA_KICK);

        tiempoKick = millis();
        kickActivo = true;

        direccionActual = DERECHA;
        return;
    }

    analogWrite(motorDelDer,FUERZA_MANTENIMIENTO);
    analogWrite(motorDelIzq,0);

    direccionActual = DERECHA;
    ultimaDireccion = DERECHA;
}



//==================================================
//      ACTUALIZAR KICKSTART
//==================================================

void actualizarKickstart()
{
    if(!kickActivo)
        return;

    if(millis() - tiempoKick >= TIEMPO_KICK)
    {
        kickActivo = false;

        if(direccionActual == IZQUIERDA)
        {
            analogWrite(motorDelIzq,FUERZA_MANTENIMIENTO);
        }

        if(direccionActual == DERECHA)
        {
            analogWrite(motorDelDer,FUERZA_MANTENIMIENTO);
        }
    }
}