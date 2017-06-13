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

class QTimerEventTest : public QObject {
public:
    QTimerEventTest() :
        m_maxPeriod( 0 )
      , m_logFile( ( "../../qtimer-signal-slot - " + QDateTime::currentDateTime().toString( "dd-MM-yyyy,hh-mm-ss" ).toUtf8() + ".txt" ) )
      , m_timer()
    {
        if( ! m_logFile.open( QFile::ReadWrite | QIODevice::Text ) ) {
            qFatal( "Erro ao abrir arquivo de log." );
        }

        m_timer.setInterval( PERIOD );
        m_timer.start();
        QObject::connect( &m_timer, &QTimer::timeout, this, &QTimerEventTest::periodicCall );
    }

    ~QTimerEventTest() {
        m_logFile.flush();
        m_logFile.close();
    }

    void periodicCall() {
        /*--- Variaveis estáticas para medição de tempo ---*/
        static TimeElement now = std::chrono::system_clock::now();
        static TimeElement last = std::chrono::system_clock::now();

        /*-- Executando mediçao de tempo em us. ---*/
        now = std::chrono::system_clock::now();

        uint32_t loopDur = std::chrono::duration_cast< std::chrono::microseconds >( now - last ).count();

       m_logFile.write( (QString::number(loopDur) + "\n").toUtf8().constData()  );

        if( loopDur > m_maxPeriod ) {
            qCritical() << "Novo máximo: " << loopDur/1000.0;

            m_maxPeriod = loopDur;
        }

        /*-- Preparando variável para proxima chamada. ---*/
        last = now;
    }

private:
    uint32_t m_maxPeriod;
    QFile m_logFile;

    /*--- Configurando o Timer do Windows que esta sendo utilizado para testes. ---*/
    QTimer m_timer;

};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    /*--- Timer para finalização do Software. ---*/
    QTimer::singleShot( TEST_DURATION*1000, &a, &QCoreApplication::quit );

    /*--- Classe que Recebe ---*/
    QTimerEventTest event;

    /*--- Iniciando o loop de eventos. ----*/
    return a.exec();
}
