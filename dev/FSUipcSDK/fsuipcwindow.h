#ifndef FSUIPCWINDOW_H
#define FSUIPCWINDOW_H

#include <memory>

#include <QMainWindow>
#include <QThread>

#include <QtCharts/QChartView>
#include <QtCharts/QChart>

#include <QHBoxLayout>

QT_CHARTS_BEGIN_NAMESPACE
class QSplineSeries;
class QValueAxis;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

#include "fsinterface.h"

struct ChartElements {
    QChartView *m_chartView;
    QChart *m_chart;

    QSplineSeries *m_pitchSeries;
    QSplineSeries *m_rollSeries;
    QSplineSeries *m_yawSeries;

    QStringList m_titles;

    QValueAxis *m_axis;
};

namespace Ui {
class FSUIPcWindow;
}

class FSUIPcWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FSUIPcWindow(QWidget *parent = 0);
    ~FSUIPcWindow();

private:
    void createThreeAxisPlot(ChartElements &cs , const QString &title);

    void imuUpdated( const IMUData &newdata );
    void positionUpdated( const GPSData &newdata );

    /*--- Metodos para atualizacao do chart. ---*/
    void addPointToChart( ChartElements &cs, QSplineSeries &series, double x, double y, bool scroll = false );

    Ui::FSUIPcWindow *ui;

    /*-- Clases. ---*/
    std::unique_ptr< FSInterface > m_interface;

    ChartElements m_attitudeCharts;
    ChartElements m_gyroCharts;

    double m_lastChartData;

    QThread *m_interfaceThread;
};

#endif // FSUIPCWINDOW_H
