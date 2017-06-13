#include <algorithm>
#include <iostream>
#include <chrono>

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QFile>

/*--- Typedefs. ---*/
using TimeElement = std::chrono::time_point<std::chrono::system_clock> ;

/*--- Configurando constantes do software. ---*/
static const uint16_t PERIOD = 20; //ms
static const uint16_t TEST_DURATION = 600; //Segundos

/*--- Função basica para utilizando um QTimerEvento. ---*/
class TimerEventTest : public QObject {
public:
    TimerEventTest() :
        m_outputFile( ( "../../qt-event-timer - " + QDateTime::currentDateTime().toString( "dd-MM-yyyy,hh-mm-ss" ).toUtf8() + ".txt" ) )
      , m_timerId( -1 )
      , m_quitId( -1 )
      , m_maxPeriod( 0 )
    {
        m_timerId = startTimer( PERIOD, Qt::PreciseTimer );
        m_quitId = startTimer( TEST_DURATION*1000);

        if( m_timerId == 0 ) {
            qFatal( "Erro ao iniciar timer periodico" );
        }

        if( m_quitId == 0 ) {
            qFatal( "Erro ao iniciar timer de finalizacao");
        }

        if( ! m_outputFile.open( QFile::ReadWrite | QIODevice::Text ) ) {
            qFatal( "Erro ao abrir arquivo de log." );
        }

    }

    ~TimerEventTest() {
        qCritical() << Q_FUNC_INFO;

        killTimer( m_timerId );

        m_outputFile.flush();
        m_outputFile.close();
    }

    void timerEvent( QTimerEvent *e ) {
        if( m_timerId == e->timerId() ) {

            static TimeElement now = std::chrono::system_clock::now();
            static TimeElement last = std::chrono::system_clock::now();

            /*-- Executando mediçao de tempo em us. ---*/
            now = std::chrono::system_clock::now();

            uint32_t loopDur = std::chrono::duration_cast< std::chrono::microseconds >( now - last ).count();

            m_outputFile.write( (QString::number(loopDur) + "\n").toUtf8().constData()  );

            if( loopDur > m_maxPeriod ) {
                qCritical() << "Novo máximo: " << loopDur/1000.0;

                m_maxPeriod = loopDur;
            }

            //double periodError = PERIOD - loopDur/1000.0f;
            //qCritical() << loopDur/1000.0f << ", " << periodError;

            /*-- Preparando variável para proxima chamada. ---*/
            last = now;

        }else if( m_quitId == e->timerId() ) {
            QCoreApplication::quit();
        }
    }

private:

    QFile m_outputFile;

    int m_timerId;
    int m_quitId;

    uint32_t m_maxPeriod;

};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TimerEventTest e;

    return a.exec();
}
