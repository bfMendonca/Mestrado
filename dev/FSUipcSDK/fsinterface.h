#ifndef FSINTERFACE_H
#define FSINTERFACE_H

/*--- Includes do Windows. --*/
#include "Windows.h"

/*--- Includes da Std. ---*/
#include <chrono>

/*--- Includes do Qt. ---*/
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QObject>
#include <QTimer>
#include <QTimerEvent>

/*--- Arquivos do projeto. ---*/
#include "fsuipckeyfile.h"

/*--- Includes do FS UIpc ---*/
#include "FSUIPC_User.h"
#include "resource.h"

/*--- Definicoes de variaveis. ---*/
using TimeType = std::chrono::system_clock::time_point;


//#define DEBUG_TIME

#define LOG_PERIOD
#define LOG_FSUIPC_PERIOD

#if defined( LOG_FSUIPC_PERIOD ) || defined( LOG_PERIOD )
struct TimeMeasStruct{
    TimeType n1;
    TimeType n2;

    bool initialized;
};
#endif

/*--- Estrutura simples para agrumento de dados, melhor no futuro. ---*/
struct Sensors {
    double x, y, z;
};

struct IMUData {

    Sensors orientation;
    Sensors angularVelocity;
    Sensors linearAcceleration;

    double time;

};

struct GPSData {
    double lat, lon;
    double altitude;
    double course;
    double speed;

    double time;
};

class FSInterface : public QObject
{
    Q_OBJECT
public:
    explicit FSInterface(QObject *parent = 0);
    ~FSInterface();

signals:
    void imuUpdate( const IMUData &p );
    void positionUpdated( const GPSData &p );

private:
    /**
     * @brief getData
     * Responsável por retirar os dados da simulaçao através da interface FSUIPC
     */
    void getData();

    /**
     * @brief timerEvent
     * Call back de timers do Qt.
     * @param e
     */
    void timerEvent( QTimerEvent *e );


    /*---- Variáveis. ---*/
    int m_updateTimerEvent;

    /*--- Variaveis envolvidas com a FSUIPc. ---*/
    unsigned long int m_result;
    uint8_t m_buffer[15000];

    /*--- Estruturas com os dados recuperados da simulação. ---*/
    IMUData m_lastRcvdImu;
    GPSData m_lastRcvdPos;

    /*--- Estrutura para armazenamento de tempo. ---*/
    TimeType m_lastUpdateTime;
    const TimeType m_start;

    /*--- Constantes. ---*/
    static const int PERIOD = 20;

    /*--- A partir daqui temos variaveis que nao necessariamente estarao ativiadas em atividade normal. ---*/

    //QFile m_globalPeriodLogFile;

    /*--- Estruturas para medicao de tempo. ---*/
#if defined( LOG_FSUIPC_PERIOD )
    QFile m_fsuipcPeriodsLogFile;
    TimeMeasStruct m_fspuicTimeMeasStruct;
#endif

#if defined( LOG_PERIOD )
    QFile m_loopPeriodsLogFile;
    TimeMeasStruct m_loopPeriodsTimeMeasStruct;
#endif
};

#endif // FSINTERFACE_H
