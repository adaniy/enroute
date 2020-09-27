/***************************************************************************
 *   Copyright (C) 2019 by Stefan Kebekus                                  *
 *   stefan.kebekus@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "Meteorologist.h"
#include "WeatherReport.h"

#include <QQmlEngine>
#include <QDateTime>
#include <cmath>

WeatherReport::WeatherReport(QObject *parent) : QObject(parent)
{
}

WeatherReport::WeatherReport(const QString &id,
                             Meteorologist::METAR *metar,
                             Meteorologist::TAF *taf,
                             QObject *parent) : QObject(parent), _id(id)
{
    _metar = metar;
    if (!_metar.isNull()) {
        connect(_metar, &QObject::destroyed, this, &WeatherReport::metarChanged);
        connect(_metar, &QObject::destroyed, this, &WeatherReport::autodestruct);
    }

    _taf = taf;
    if (!_taf.isNull()) {
        connect(_taf, &QObject::destroyed, this, &WeatherReport::autodestruct);
    }
}


QGeoCoordinate WeatherReport::location() const
{
    if (!_metar.isNull())
        return _metar->coordinate();
    if (!_taf.isNull())
        return _taf->coordinate();
    return QGeoCoordinate();
}


QString WeatherReport::station_id() const
{
    if (!_metar.isNull())
        return _metar->ICAOCode();
    if (!_taf.isNull())
        return _taf->ICAOCode();
    return QString();

}


int WeatherReport::qnh() const
{
    if (_metar.isNull())
        return 0;
    return _metar->QNH();
}


void WeatherReport::autodestruct()
{
    qWarning() << station_id() << "Autodestruct";
    if (_metar.isNull() && _taf.isNull())
        deleteLater();
}


QString WeatherReport::decodeTime(const QVariant &time) {
    QDateTime tim = QDateTime::fromString(time.toString().replace("T", " "), "yyyy-MM-dd hh:mm:ssZ");
    return tim.toString("ddd MMMM d yyyy hh:mm") + " UTC";
}


QString WeatherReport::decodeWind(const QVariant &windd, const QVariant &winds, const QVariant &windg) {
    QString w;
    if (windd.toString() == "0")
        if (winds.toString() == "0")
            return "calm";
        else
            w += "variable";
    else
        w += "from " + windd.toString() + "°";
    w += " at " + winds.toString() + " kt";
    if (windg.toString() != "0")
        w+= ", gusty " + windg.toString() + " kt";
    return w;
}


QString WeatherReport::decodeVis(const QVariant &vis) {
    long v = std::lround(vis.toString().toDouble() * 1.61);
    return QString::number(v) + " km";
}


QString WeatherReport::decodeTemp(const QVariant &temp) {
    QString tmp = temp.toString();
    return tmp.left(tmp.lastIndexOf(".")) + " °C";
}


QString WeatherReport::decodeQnh(const QVariant &altim) {
    long qnh = std::lround(altim.toString().toDouble() * 33.86);
    return QString::number(qnh) + " hPa";
}


QString WeatherReport::decodeWx(const QVariant &wx) {
    QString w = wx.toString();
    // clear
    w.replace("NSW", "No significant weather");
    // intensity
    w.replace("-", "light ");
    w.replace("+", "heavy ");
    w.replace("VC", "in the vicinity ");
    w.replace("RE", "recent ");
    // qualifier
    w.replace("BC", "patches of");
    w.replace("BL", "blowing");
    w.replace("FZ", "freezing");
    w.replace("MI", "shallow");
    w.replace("PR", "partial");
    w.replace("RE", "recent");
    w.replace("SH", "showers of");
    // precipitation
    w.replace("DZ", "drizzle");
    w.replace("IC", "ice crystal");
    w.replace("GR", "hail");
    w.replace("GS", "snow pellets");
    w.replace("PL", "ice pellets");
    w.replace("RA", "rain");
    w.replace("SN", "snow");
    w.replace("SG", "snow grains");
    // obscuration
    w.replace("BR", "mist");
    w.replace("DU", "dust");
    w.replace("FG", "fog");
    w.replace("FU", "smoke");
    w.replace("HZ", "haze");
    w.replace("PY", "spray");
    w.replace("SA", "sand");
    w.replace("VA", "volcanic ash");
    // other
    w.replace("DS", "duststorm");
    w.replace("FC", "tornado");
    w.replace("TS", "thunderstorm");
    w.replace("SQ", "squalls");
    w.replace("SS", "sandstorm");
    return w;
}


QString WeatherReport::decodeClouds(const QVariantList &clouds) {
    QString clds;
    for (int i = clouds.size() - 1; i >= 0; --i) {
        QList<QString> layer = clouds[i].toString().split(",");
        clds += layer[0];
        if (layer.size() >= 2) {
            if (layer.size() == 3)
                clds += " " + layer[2];
            else
                clds += " clouds";
            clds += " at " + layer[1] + " ft AGL";
        }
        if (i > 0)
            clds += "<br>";
    }
    clds.replace("NSC", "No significant clouds");
    clds.replace("SKC", "Sky clear");
    clds.replace("CLR", "Clear");
    clds.replace("CAVOK", "Ceiling and visibility OK");
    clds.replace("FEW", "Few");
    clds.replace("SCT", "Scattered");
    clds.replace("BKN", "Broken");
    clds.replace("OVC", "Overcast");
    clds.replace("OVX", "Obscured");
    clds.replace("OVCX", "Obscured");
    clds.replace("CB", "Cumulonimbus");
    clds.replace("TCU", "Towering cumulus");
    clds.replace("CU", "Cumulus");
    return clds;
}


QString WeatherReport::oneLineDescription() const {

    if (_metar)
        return _metar->summary();
    else
        qWarning() << station_id() << "No METAR";
    return QString();
}


void WeatherReport::setClock(Clock *clock)
{
    if (!_clock.isNull())
        disconnect(_clock, &Clock::timeChanged, this, &WeatherReport::richTextNameChanged);

    _clock = clock;

    if (!_clock.isNull())
        connect(_clock, &Clock::timeChanged, this, &WeatherReport::richTextNameChanged);
}


void WeatherReport::setSatNav(SatNav *satNav)
{
    if (!_satNav.isNull()) {
        disconnect(_satNav, &SatNav::statusChanged, this, &WeatherReport::richTextNameChanged);
        disconnect(_satNav, &SatNav::update, this, &WeatherReport::richTextNameChanged);
    }

    _satNav = satNav;

    if (!_satNav.isNull()) {
        connect(_satNav, &SatNav::statusChanged, this, &WeatherReport::richTextNameChanged);
        connect(_satNav, &SatNav::update, this, &WeatherReport::richTextNameChanged);
    }
}


QString WeatherReport::richTextName() const
{
    return "Arglebargle";

    /*
                {
                    var result = model.modelData.id
                    var wp = geoMapProvider.findByID(model.modelData.id)
                    if (wp !== null) {
                        // Mention items that will lead to change of text
                        satNav.status
                        satNav.lastValidCoordinate

                        return wp.richTextName
                    }
                    if (satNav.status === SatNav.OK)
                        result += "<br>" + satNav.wayTo(model.modelData.location, globalSettings.useMetricUnits)
                    return result
                }
    */
}