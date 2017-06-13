#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QTimer>

/*--- Typedefs. ---*/
using TimeElement = std::chrono::time_point<std::chrono::system_clock> ;

/*--- Configurando constantes do software. ---*/
static const uint16_t PERIOD = 20; //ms
static const uint16_t TEST_DURATION = 600; //Segundos

uint32_t maxPeriod = 0;
QFile logFile;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    /*--- Abertura do arquivo de log. ---*/
    logFile.setFileName( ( "../../qtimer-lambda - " + QDateTime::currentDateTime().toString( "dd-MM-yyyy,hh-mm-ss" ).toUtf8() + ".txt" ) );

    if( ! logFile.open( QFile::ReadWrite | QIODevice::Text ) ) {
        qFatal( "Erro ao abrir arquivo de log." );
    }
    /*--- Timer para finalização do Software. ---*/
    QTimer::singleShot( TEST_DURATION*1000, &a, &QCoreApplication::quit );
    QObject::connect( &a, &QCoreApplication::aboutToQuit, &logFile, &QFile::flush );
    QObject::connect( &a, &QCoreApplication::aboutToQuit, &logFile, &QFile::close );

    /*--- Configurando o Timer do Windows que esta sendo utilizado para testes. ---*/
    QTimer timer;
    timer.setInterval( PERIOD );
    QObject::connect( &timer, &QTimer::timeout, [=]() {
        /*--- Variaveis estáticas para medição de tempo ---*/
        static TimeElement now = std::chrono::system_clock::now();
        static TimeElement last = std::chrono::system_clock::now();

        /*-- Executando mediçao de tempo em us. ---*/
        now = std::chrono::system_clock::now();

        uint32_t loopDur = std::chrono::duration_cast< std::chrono::microseconds >( now - last ).count();

       logFile.write( (QString::number(loopDur) + "\n").toUtf8().constData()  );

        if( loopDur > maxPeriod ) {
            qCritical() << "Novo máximo: " << loopDur/1000.0;

            maxPeriod = loopDur;
        }

        /*-- Preparando variável para proxima chamada. ---*/
        last = now;

    } );

    timer.start();

    /*--- Iniciando o loop de eventos. ----*/
    return a.exec();
}
