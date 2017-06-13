#include "fsuipcwindow.h"
#include "ui_fsuipcwindow.h"

#include <QtCharts/QAbstractAxis>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>

FSUIPcWindow::FSUIPcWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FSUIPcWindow)
  , m_interface( new FSInterface( ) )
  , m_lastChartData( 0.0 )
  , m_interfaceThread( new QThread( this ) )
{
    ui->setupUi(this);

    /*--- Configurações para mover a classe que gerencia a interface para outra Thread. ---*/
    m_interface->moveToThread( m_interfaceThread );

    connect( m_interfaceThread, &QThread::finished, m_interface.get(), &FSInterface::deleteLater );
    connect( m_interface.get(), &FSInterface::destroyed, m_interfaceThread, &QThread::deleteLater );
    connect( m_interfaceThread, &QThread::destroyed, [=]() {
        qCritical() << Q_FUNC_INFO << " Finalização de Thread";
    });

    /*--- Conexao de dados. ---*/
    connect( m_interface.get(), &FSInterface::imuUpdate, this, &FSUIPcWindow::imuUpdated, Qt::QueuedConnection );
    connect( m_interface.get(), &FSInterface::positionUpdated, this, &FSUIPcWindow::positionUpdated, Qt::QueuedConnection );

    m_interfaceThread->start( QThread::TimeCriticalPriority );

    /*--- Inserindo itens na interface Gráfica. ---*/
    /*--- Aqui estamos montando os gráficos que serão exibidos. ---*/
    createThreeAxisPlot( m_attitudeCharts, "Orientação" );
    createThreeAxisPlot( m_gyroCharts, "Velocidades angulares" );

    /*--- Inserindo os componentes gerados no layout. ---*/
    QHBoxLayout *attLayout = new QHBoxLayout( ui->attitudeGraphsTab );
    attLayout->addWidget( m_attitudeCharts.m_chartView );

    QHBoxLayout *gyroLayout = new QHBoxLayout( ui->gyroGraphsTab );
    gyroLayout->addWidget( m_gyroCharts.m_chartView );
}

FSUIPcWindow::~FSUIPcWindow()
{
    m_interfaceThread->quit();
    m_interfaceThread->wait();

    delete ui;
}

void FSUIPcWindow::createThreeAxisPlot(ChartElements &cs, const QString & title )
{

    cs.m_chart = new QChart;
    cs.m_chartView = new QChartView( cs.m_chart );
    cs.m_chartView->setMinimumSize( 800, 600 );

    /*--- Criando as series que serão exibidas. ---*/
    cs.m_pitchSeries = new QSplineSeries(this);
    cs.m_rollSeries = new QSplineSeries(this);
    cs.m_yawSeries = new QSplineSeries(this);

    /*--- Adicionando series aos graficos. ---*/
    cs.m_chart->addSeries( cs.m_pitchSeries );
    cs.m_chart->addSeries( cs.m_rollSeries );
    cs.m_chart->addSeries( cs.m_yawSeries );

    cs.m_pitchSeries->setName( "Pitch" );
    cs.m_rollSeries->setName( "Roll" );
    cs.m_yawSeries->setName( "Yaw" );

    cs.m_chart->setTitle( title );

    /*--- Criando os eixos do gráfico. ---*/
    QValueAxis *axisX = new QValueAxis;
    axisX->setRange(0, 15000);
    axisX->setLabelFormat("%g");
    axisX->setTitleText("[ms]");

    QValueAxis *axisY = new QValueAxis;
    axisY->setRange(-45, 45);
    axisY->setTitleText("Angle [deg]");

    /*--- Ajustando as cores dos graficos. ---*/
    QPen green( Qt::green );
    green.setWidth( 3 );

    QPen red( Qt::red );
    red.setWidth( 3 );

    QPen blue( Qt::blue );
    blue.setWidth( 3 );

    cs.m_pitchSeries->setPen( green );
    cs.m_rollSeries->setPen( red );
    cs.m_yawSeries->setPen( blue );

    /*--- Atrelando as séries aos eixos. ---*/
    cs.m_chart->setAxisX( axisX );
    cs.m_chart->setAxisY( axisY );

    cs.m_pitchSeries->attachAxis( axisX );
    cs.m_pitchSeries->attachAxis( axisY );

    cs.m_rollSeries->attachAxis( axisX );
    cs.m_rollSeries->attachAxis( axisY );

    cs.m_yawSeries->attachAxis( axisX );
    cs.m_yawSeries->attachAxis( axisY );
}

void FSUIPcWindow::imuUpdated(const IMUData &newdata)
{
    Q_UNUSED( newdata )

    if( ( newdata.time - m_lastChartData ) > 100 ) {

        /*--- Atualizando os "Charts. ---*/
        addPointToChart( m_attitudeCharts, *m_attitudeCharts.m_pitchSeries, newdata.time, newdata.orientation.x, true );
        addPointToChart( m_attitudeCharts, *m_attitudeCharts.m_rollSeries, newdata.time, newdata.orientation.y, false );
        addPointToChart( m_attitudeCharts, *m_attitudeCharts.m_yawSeries, newdata.time, newdata.orientation.z, false );

        addPointToChart( m_gyroCharts, *m_gyroCharts.m_pitchSeries, newdata.time, newdata.angularVelocity.x, true );
        addPointToChart( m_gyroCharts, *m_gyroCharts.m_rollSeries, newdata.time, newdata.angularVelocity.y, false );
        addPointToChart( m_gyroCharts, *m_gyroCharts.m_yawSeries, newdata.time, newdata.angularVelocity.z, false );

        /*--- Atualizando o sumário. ---*/
        ui->pitchLineEdit->setText( QString::number( newdata.orientation.x, 'g', 3 ) );
        ui->rollLineEdit->setText( QString::number( newdata.orientation.y, 'g', 3 ) );
        ui->yawLineEdit->setText( QString::number( newdata.orientation.z, 'g', 3 ) );

        m_lastChartData = newdata.time;
    }
}

void FSUIPcWindow::positionUpdated(const GPSData &newdata)
{
    Q_UNUSED( newdata )

    /*--- Atualizando o sumário. ---*/
    ui->latLineEdit->setText( QString::number( newdata.lat, 'g', 7 ) );
    ui->lonLineEdit->setText( QString::number( newdata.lon, 'g', 7 ) );
    ui->altLineEdit->setText( QString::number( newdata.altitude, 'g', 7 ) );
}

void FSUIPcWindow::addPointToChart( ChartElements &cs, QSplineSeries &series, double x, double y, bool scroll)
{
    QVector<QPointF> points( series.pointsVector() );
    points.push_back( QPointF( x, y ) );

    qint64 maxSize = 15000;

    if( scroll ) {
        if(  x > maxSize ) {
            double dx = points.at( 1 ).x() - points.at( 0 ).x();
            dx *= cs.m_chart->plotArea().width()/15000.0;
            cs.m_chart->scroll( dx, 0 );

            points.removeAt(0);
        }
    }

    series.replace( points );
}
