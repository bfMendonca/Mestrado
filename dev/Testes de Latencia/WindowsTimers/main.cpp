#include <algorithm>
#include <iostream>
#include <chrono>

#include <Windows.h>
#include <Mmsystem.h>

#include <QDateTime>
#include <QDebug>
#include <QFile>

/*--- Typedefs. ---*/
using TimeElement = std::chrono::time_point<std::chrono::system_clock> ;

/*--- Configurando constantes do software. ---*/
static const uint16_t PERIOD = 20; //ms
static const uint16_t TEST_DURATION = 600; //Segundos

uint32_t maxPeriod = 0;

void CALLBACK TimerProc( PVOID lpParam, BOOLEAN TimerOrWaitFired ) {
    Q_UNUSED( lpParam )
    Q_UNUSED( TimerOrWaitFired )

    static TimeElement now = std::chrono::system_clock::now();
    static TimeElement last = std::chrono::system_clock::now();

    /*-- Executando mediçao de tempo em us. ---*/
    now = std::chrono::system_clock::now();

    uint32_t loopDur = std::chrono::duration_cast< std::chrono::microseconds >( now - last ).count();

    QFile *file = reinterpret_cast< QFile * >( lpParam );

    file->write( (QString::number(loopDur) + "\n").toUtf8().constData()  );

    if( loopDur > maxPeriod ) {
        qCritical() << "Novo máximo: " << loopDur/1000.0;

        maxPeriod = loopDur;
    }

    //double periodError = PERIOD - loopDur/1000.0f;
    //qCritical() << loopDur/1000.0f << ", " << periodError;

    /*-- Preparando variável para proxima chamada. ---*/
    last = now;
}

int main(int argc, char *argv[])
{
    Q_UNUSED( argc )
    Q_UNUSED( argv )

    /*--- Abertura do arquivo de log. ---*/
    QFile logFile( "../../windows-timers - " + QDateTime::currentDateTime().toString( "dd-MM-yyyy,hh-mm-ss" ).toUtf8() + ".txt" );

    if( ! logFile.open( QFile::ReadWrite | QIODevice::Text ) ) {
        qFatal( "Erro ao abrir arquivo de log." );
    }

    /*--- Configurando o Timer do Windows que esta sendo utilizado para testes. ---*/

    HANDLE timerHandle;
    CreateTimerQueueTimer( &timerHandle, NULL, TimerProc, &logFile, 0, PERIOD, WT_EXECUTEINTIMERTHREAD );


    /*--- Software permenece enquanto os testes estão sendo executados. ---*/
    bool running = true;

    auto start = std::chrono::system_clock::now();
    auto end = start;

    while( running ){

        end = std::chrono::system_clock::now();

        if( std::chrono::duration_cast< std::chrono::seconds >( end - start ).count() < TEST_DURATION ) {
            Sleep( 100 );
        }else {
            running = false;
        }
    }

    /*--- Fechamento do arquivo de log. ---*/
    logFile.flush();
    logFile.close();

    return 0;
}
