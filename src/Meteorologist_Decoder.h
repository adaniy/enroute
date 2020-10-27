/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

#pragma once

#include "../3rdParty/metaf/include/metaf.hpp"

#include "Meteorologist.h"

using namespace metaf;


/*!\brief METAR/TAF decoder
 *
 * This class takes METAR or TAF messages in raw form and converts them to human-
 * readable, translated text.
 */

class Meteorologist::Decoder : public QObject, public metaf::Visitor<QString> {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     * This constructor creates a Decoder instance.  You need to set the raw text
     * before this class can be useful.
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Decoder(QObject *parent = nullptr);

    /*! Decoded text of the METAR/TAF message
     *
     * This property holds the decoded text of the message, as a human-readable,
     * rich text string.  The text might change in responde to changes in
     * user settings, and might also change by midnight (the text uses words such
     * as 'tomorrow' whose meaning changes at the end of the day).
     */
    Q_PROPERTY(QString decodedText READ decodedText NOTIFY decodedTextChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property decodedText
     */
    QString decodedText() const
    {
        return _decodedText;
    }

    /*! Raw text of the METAR/TAF message */
    Q_PROPERTY(QString rawText READ rawText NOTIFY rawTextChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property rawText
     */
    QString rawText() const
    {
        return _rawText;
    }

    /*! \brief Setter function for property with the same name
     *
     * @param Raw METAR/TAF message
     *
     * @param Reference date. Since METAR/TAF messages specify points in time only by "day of month" and "time",
     * the decoder needs to know the month and year. Set this reference date to any date between in the interval [issue date, issue date + 28 days]
     */
#warning needs to become protected
    void setRawText(QString rawText, QDate referenceDate);

#warning needs to become protected
    bool hasParseError() const
    {
        return (parseResult.reportMetadata.error != metaf::ReportError::NONE);
    }

    // -----------------------------------------



signals:
    // Notifier signal
    void decodedTextChanged();

    // Notifier signal
    void rawTextChanged();

private slots:
    // This slot does the actual parsing
    void parse();

private:
    // Explanation functions
    static QString explainCloudType(const metaf::CloudType ct);
    static QString explainDirection(const metaf::Direction & direction, bool trueCardinalDirections=true);
    static QString explainDirectionSector(const std::vector<metaf::Direction> dir);
    static QString explainDistance(const metaf::Distance & distance);
    static QString explainDistance_FT(const metaf::Distance & distance);
    QString explainMetafTime(const metaf::MetafTime & metafTime);
    static QString explainPrecipitation(const metaf::Precipitation & precipitation);
    static QString explainPressure(const metaf::Pressure & pressure);
    static QString explainRunway(const metaf::Runway & runway);
    static QString explainSpeed(const metaf::Speed & speed);
    static QString explainSurfaceFriction(const metaf::SurfaceFriction & surfaceFriction);
    static QString explainTemperature(const metaf::Temperature & temperature);
    static QString explainWaveHeight(const metaf::WaveHeight & waveHeight);
    QString explainWeatherPhenomena(const metaf::WeatherPhenomena & wp);

    // … toString Methods
    static QString brakingActionToString(metaf::SurfaceFriction::BrakingAction brakingAction);
    static QString cardinalDirectionToString(metaf::Direction::Cardinal cardinal);
    static QString cloudAmountToString(metaf::CloudGroup::Amount amount);
    static QString cloudHighLayerToString(metaf::LowMidHighCloudGroup::HighLayer highLayer);
    static QString cloudLowLayerToString(metaf::LowMidHighCloudGroup::LowLayer lowLayer);
    static QString cloudMidLayerToString(metaf::LowMidHighCloudGroup::MidLayer midLayer);
    static QString cloudTypeToString(metaf::CloudType::Type type);
    static QString convectiveTypeToString(metaf::CloudGroup::ConvectiveType type);
    static QString distanceMilesFractionToString(metaf::Distance::MilesFraction f);
    static QString distanceUnitToString(metaf::Distance::Unit unit);
    static QString layerForecastGroupTypeToString(metaf::LayerForecastGroup::Type type);
    static QString pressureTendencyTrendToString(metaf::PressureTendencyGroup::Trend trend);
    static QString pressureTendencyTypeToString(metaf::PressureTendencyGroup::Type type);
    static QString probabilityToString(metaf::TrendGroup::Probability prob);
    static QString runwayStateDepositsToString(metaf::RunwayStateGroup::Deposits deposits);
    static QString runwayStateExtentToString(metaf::RunwayStateGroup::Extent extent);
    static QString specialWeatherPhenomenaToString(const metaf::WeatherPhenomena & wp);
    static QString stateOfSeaSurfaceToString(metaf::WaveHeight::StateOfSurface stateOfSurface);
    static QString visTrendToString(metaf::VisibilityGroup::Trend trend);
    static QString weatherPhenomenaDescriptorToString(metaf::WeatherPhenomena::Descriptor descriptor);
    static QString weatherPhenomenaQualifierToString(metaf::WeatherPhenomena::Qualifier qualifier);
    static QString weatherPhenomenaWeatherToString(metaf::WeatherPhenomena::Weather weather);

    // visitor Methods
    virtual QString visitCloudGroup(const CloudGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitCloudTypesGroup(const CloudTypesGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitKeywordGroup(const KeywordGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitLayerForecastGroup(const LayerForecastGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitLightningGroup(const LightningGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitLocationGroup(const LocationGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitLowMidHighCloudGroup(const LowMidHighCloudGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitMinMaxTemperatureGroup(const MinMaxTemperatureGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitMiscGroup(const MiscGroup & group,  ReportPart reportPart, const std::string & rawString);
    virtual QString visitPressureGroup(const PressureGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitPressureTendencyGroup(const PressureTendencyGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitReportTimeGroup(const ReportTimeGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitPrecipitationGroup(const PrecipitationGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitRunwayStateGroup(const RunwayStateGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitSeaSurfaceGroup(const SeaSurfaceGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitTemperatureGroup(const TemperatureGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitTrendGroup(const TrendGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitUnknownGroup(const UnknownGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitVicinityGroup(const VicinityGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitVisibilityGroup(const VisibilityGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitWeatherGroup(const WeatherGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitWindGroup(const WindGroup & group, ReportPart reportPart, const std::string & rawString);


#warning docu
    QString _decodedText;
    QString _rawText;
    QDate _referenceDate;

    // Result of the parser
    ParseResult parseResult;
};

