#include "fsinterface.h"

FSInterface::FSInterface(QObject *parent) : QObject(parent)
  , m_updateTimerEvent( -1 )
  , m_start( std::chrono::system_clock::now() )
{
    qRegisterMetaType< IMUData >("IMUData");
    qRegisterMetaType< GPSData >("GPSData");

    m_updateTimerEvent = startTimer( PERIOD, Qt::PreciseTimer );

    /*--- Abrinco conexão com FSUIPC. ---*/
    if( ( FSUIPC_Open( SIM_ANY, &m_result ) != TRUE ) ||
        ( FSUIPC_Write(0x8001, 12, FSUIPCKey::chOurKey, &m_result ) != TRUE ) ||
        ( FSUIPC_Process( &m_result) != TRUE ) )  {

        qFatal( "Erro ao abrir a comunicação" );

    }

#if defined(LOG_FSUIPC_PERIOD)
    /*--- Preparando mecanismos para logar tempo, caso necessario. ---*/
    m_fsuipcPeriodsLogFile.setFileName(  "../../fsuipcperiod- "
                                         + QDateTime::currentDateTime().toString( "dd-MM-yyyy,hh-mm-ss" ).toUtf8()
                                         + ".txt");

    if( !m_fsuipcPeriodsLogFile.open(  QFile::ReadWrite | QIODevice::Text ) ) {
        qFatal( "Erro ao abrir arquivo de log" );
    }
#endif

#if defined(LOG_PERIOD)
    /*--- Preparando mecanismos para logar tempo, caso necessario. ---*/
    m_loopPeriodsLogFile.setFileName(  "../../loop-period- "
                                         + QDateTime::currentDateTime().toString( "dd-MM-yyyy,hh-mm-ss" ).toUtf8()
                                         + ".txt");

    if( !m_loopPeriodsLogFile.open(  QFile::ReadWrite | QIODevice::Text ) ) {
        qFatal( "Erro ao abrir arquivo de log" );
    }

    m_loopPeriodsTimeMeasStruct.initialized = false;
#endif
}

FSInterface::~FSInterface()
{
    qCritical() << Q_FUNC_INFO;
    FSUIPC_Close();

#if defined( LOG_FSUIPC_PERIOD )
    m_fsuipcPeriodsLogFile.flush();
    m_fsuipcPeriodsLogFile.close();
#endif

#if defined( LOG_PERIOD )
    m_loopPeriodsLogFile.flush();
    m_loopPeriodsLogFile.close();
#endif
}

void FSInterface::getData()
{

#if defined( LOG_PERIOD )
    m_loopPeriodsTimeMeasStruct.n2 = std::chrono::system_clock::now();
    uint32_t loopPeriod = std::chrono::duration_cast< std::chrono::microseconds >( m_loopPeriodsTimeMeasStruct.n2 - m_loopPeriodsTimeMeasStruct.n1 ).count();
    m_loopPeriodsTimeMeasStruct.n1 = m_loopPeriodsTimeMeasStruct.n2;

    if( m_loopPeriodsTimeMeasStruct.initialized )
        m_loopPeriodsLogFile.write( (QString::number(loopPeriod) + "\n").toUtf8().constData()  );
    else
        m_loopPeriodsTimeMeasStruct.initialized = true;
#endif

#if defined( LOG_FSUIPC_PERIOD )
    m_fspuicTimeMeasStruct.n1 = std::chrono::system_clock::now();
#endif
    if( ( FSUIPC_Read( 0x02B4,  8, reinterpret_cast< DWORD * >(m_buffer+ 0), &m_result ) != TRUE ) ||
        ( FSUIPC_Read( 0x0560, 24, reinterpret_cast< DWORD * >(m_buffer+ 8), &m_result ) != TRUE ) ||
        ( FSUIPC_Read( 0x0578, 12, reinterpret_cast< DWORD * >(m_buffer+32), &m_result ) != TRUE ) ||
        ( FSUIPC_Read( 0x30A8, 24, reinterpret_cast< DWORD * >(m_buffer+44), &m_result ) != TRUE ) ||
        ( FSUIPC_Read( 0x3060, 24, reinterpret_cast< DWORD * >(m_buffer+68), &m_result ) != TRUE ) ||
        ( FSUIPC_Process( &m_result) != TRUE ) ) {

        qCritical() << Q_FUNC_INFO << "Falha na execução do processo";
        return;
    }

#if defined( LOG_FSUIPC_PERIOD )
    m_fspuicTimeMeasStruct.n2 = std::chrono::system_clock::now();
    uint32_t processDur = std::chrono::duration_cast< std::chrono::microseconds >( m_fspuicTimeMeasStruct.n2 - m_fspuicTimeMeasStruct.n1 ).count();
    m_fsuipcPeriodsLogFile.write( (QString::number(processDur) + "\n").toUtf8().constData()  );
#endif

    /*---- Tempo
     * Para obter tempo, uma vez que o jogo não fornece com resolução suficiente,
     * iremos pegar o tempo à partir do momento que o software foi aberto.
     * ----*/
    m_lastUpdateTime = std::chrono::system_clock::now();
    double time = std::chrono::duration_cast< std::chrono::microseconds >( m_lastUpdateTime - m_start ).count()/1000.0;

    /*--- Parseando a orientação da aeronave. ---*/
    {
        int32_t pitch, roll, yaw;

        memcpy( &pitch, m_buffer+32, 4 );
        memcpy( &roll,  m_buffer+36, 4 );
        memcpy( &yaw,   m_buffer+40, 4 );

        memcpy( &m_lastRcvdImu.angularVelocity.x, m_buffer+44, 8 );
        memcpy( &m_lastRcvdImu.angularVelocity.y, m_buffer+52, 8 );
        memcpy( &m_lastRcvdImu.angularVelocity.z, m_buffer+60, 8 );

        m_lastRcvdImu.angularVelocity.x *= 57.3f;
        m_lastRcvdImu.angularVelocity.y *= 57.3f;
        m_lastRcvdImu.angularVelocity.z *= 57.3f;

        /*--- Conversao da leitura em "rads/s" ---*/
        m_lastRcvdImu.orientation.x = pitch*360.0/double(65536.0*65536.0);
        m_lastRcvdImu.orientation.y = roll*360.0/double(65536.0*65536.0);
        m_lastRcvdImu.orientation.z = yaw*360.0/double(65536.0*65536.0);


        /*--- Obtendo as acelerações da aeronave. ---*/
        memcpy( &m_lastRcvdImu.linearAcceleration.x, m_buffer+68, 8 );
        memcpy( &m_lastRcvdImu.linearAcceleration.y, m_buffer+76, 8 );
        memcpy( &m_lastRcvdImu.linearAcceleration.z, m_buffer+84, 8 );

        /*--- Conversão da leitura em "m/s²" ---*/
        m_lastRcvdImu.linearAcceleration.x *= 0.3048;
        m_lastRcvdImu.linearAcceleration.y *= 0.3048;
        m_lastRcvdImu.linearAcceleration.z *= 0.3048;

        m_lastRcvdImu.time = time;

        emit imuUpdate( m_lastRcvdImu );
    }

    /*--- Parseando a posição e velocidade da aeronave. ---*/
    {
        int64_t lat, lon, alt;

        /*--- Obtendo a posição da aeronave. ---*/
        memcpy( &lat, m_buffer+8, 8 );
        memcpy( &lon, m_buffer+16, 8 );
        memcpy( &alt, m_buffer+24, 8 );

        /*--- Obtendo a velocidade da aeronave. ---*/
        int32_t speed;
        memcpy( &speed,       m_buffer+0, 8 );
        m_lastRcvdPos.speed = speed*(3600.0)/(65536.)/(1852.);


        /*--- Conversão para valores. ---*/
        m_lastRcvdPos.lat = lat*double(90.0)/double(10001750.0*65536.0*65536.0);
        m_lastRcvdPos.lon=  lon*double(360.0)/double(65536.0*65536.0*65536.0*65536.0);
        m_lastRcvdPos.altitude = alt*3.28084/double(65536.0*65536.0);

        m_lastRcvdPos.course = m_lastRcvdImu.orientation.z;

        m_lastRcvdPos.time = time;

        emit positionUpdated( m_lastRcvdPos );

    }
}

void FSInterface::timerEvent(QTimerEvent *e)
{
    if( e->timerId() == m_updateTimerEvent ) {
        /*--- Interface com o flight simulator. ---*/
        getData();
    }
}
