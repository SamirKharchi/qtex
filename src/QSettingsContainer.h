#ifndef QSETTINGSCONTAINER_H
#define QSETTINGSCONTAINER_H

namespace qtex
{
   /*!
    * \class QSettingsContainer
    *
    * Container that reads, writes and deals with procedural registry data
    * This allows to fill the data container in arbitrary ways.
    * E.g. you could loop an enum or an array of strings that act as data keys.
    *
    * That way the implementer can focus on storing and retrieving the data
    * instead of the explicit registry read/write handling and also design
    * access in a consistent and more generic way.
    *
    * Example how to use it.
    * \code {.cpp}
    * enum class MyContainerIDs
    * {
    *     DataA,
    *     DataB,
    *     DataCount
    * }
    * std::vector<float> exampleData;
    *
    * EnumQSettingsContainer proceduralData("DataGroupName");
    * proceduralData.setValue(DataA, checkbox->value());
    * proceduralData.setValue(DataB, textBox->toPlainText());
    * //or
    * for(auto id = 0; id < underlying(MyContainerIDs::DataCount); ++id)
    *     proceduralData.setValue(id, exampleData[id]);
    * \endcode
    *
    * Write the container data to the registry
    * \code {.cpp}
    * proceduralData.write(settings);
    * \endcode
    *
    * Read the data back from the registry into the container
    * \code {.cpp}
    * proceduralData.read(settings);
    * \endcode
    *
    * Then use your IDs again to retrieve the data values
    * \code {.cpp}
    * checkbox->setChecked(proceduralData.value(DataA, true));
    * textBox->setPlainText(proceduralData.value(DataB, ""));
    * //or
    * for(auto id = 0; id < underlying(MyContainerIDs::DataCount); ++id)
    *     exampleData[id] = proceduralData.value(id, 0.f);
    * \endcode
    *
    * \tparam T_Key Currently can be of type: integral, QString, std::string
    */
    template < typename T_Key >
    class QSettingsContainer
    {
        using CustomDataContainer   = QMap<T_Key, QVariant>;
        using CustomDataIterator    = QMapIterator<T_Key, QVariant>;

    public:
        explicit QSettingsContainer(const QString& group) : m_group(group)
        {
            static_assert(std::is_integral<T_Key>::value ||
                            std::is_same<QString, T_Key>::value ||
                            std::is_same<std::string, T_Key>::value,
                            "Key type must be an integral, QString or std::string type.");
        }

        //! reads procedural settings of the given settings group
        void read(QSettings& settings)
        {
            settings.beginGroup(m_group);

            const QStringList keys = settings.childKeys();
            for (const auto& key : keys)
            {
                m_data[toKey<T_Key>(key)] = settings.value(key, QVariant());
            }

            settings.endGroup();
        }

        //! writes procedural settings to the given settings group
        void write(QSettings& settings) const
        {
            settings.beginGroup(m_group);

            CustomDataIterator iter(m_data);
            while (iter.hasNext())
            {
                iter.next();
                settings.setValue(fromKey<T_Key>(iter.key()), iter.value());
            }

            settings.endGroup();
        }

        //! This one handles enumeration types
        template <typename T, typename T_Id>
        T value(const T_Id index, const T& defaultValue = T(0)) const
        {
            static_assert(std::is_enum<T_Id>::value, "Passed index variable is not an enumeration type and no supported key type.");

            auto iIndex = underlying(index);
            if (!m_data.contains(iIndex))
                return defaultValue;

            return m_data.value(iIndex, defaultValue).template value<T>();
        }

        //! This one handles the supported key types
        template <typename T>
        T value(const T_Key index, const T& defaultValue = T(0)) const
        {
            if (!m_data.contains(index))
                return defaultValue;

            return m_data.value(index, defaultValue).template value<T>();
        }

        //! This one handles enumeration types
        template < typename KeyType >
        void setValue(KeyType key, const QVariant& value)
        {
            static_assert(std::is_enum<KeyType>::value, "Key is not an enumeration type and no supported key type.");

            m_data[underlying(key)] = value;
        }

        //! This one handles the supported key types
        void setValue(T_Key key, const QVariant& value)
        {
            m_data[key] = value;
        }

        //! This one handles enumeration types
        template < typename KeyType >
        bool contains(KeyType key) const
        {
            static_assert(std::is_enum<KeyType>::value, "Key is not an enumeration type and no supported key type.");
            return m_data.contains(underlying(key));
        }

        //! This one handles the supported key types
        bool contains(T_Key key) const
        {
            return m_data.contains(key);
        }

        inline const QString& getGroupName() const
        {
            return m_group;
        }

    private:
        QString m_group;
        CustomDataContainer m_data;

        //! Primary template that converts the QSettings key (QString) to the custom data key type if it's an integral
        template < typename KeyType >
        KeyType toKey(const QString& key) const
        {
            return static_cast<KeyType>(key.toInt());
        }

        //! Partial Specialization converts the QSettings key (QString) to the custom data key type if it's a QString
        template < >
        QString toKey(const QString& key) const
        {
            return key;
        }

        //! Partial Specialization converts the QSettings key (QString) to the custom data key type if it's a std::string
        template < >
        std::string toKey(const QString& key) const
        {
            return key.toStdString();
        }

        //! Primary template that converts the integral custom data key type to the QSettings key (QString)
        template < typename KeyType >
        QString fromKey(const KeyType& key) const
        {
            return QString::number(key);
        }

        //! Partial Specialization that converts a custom data key type to the QSettings key (QString)
        template < >
        QString fromKey(const QString& key) const
        {
            return key;
        }

        //! Partial Specialization that converts a std::string custom data key type to the QSettings key (QString)
        template < >
        QString fromKey(const std::string& key) const
        {
            return QString::fromStdString(s);
        }
    };
}

using EnumQSettingsContainer        = qtex::QSettingsContainer<int>;
using StringQSettingsContainer      = qtex::QSettingsContainer<QString>;
using StdStringQSettingsContainer   = qtex::QSettingsContainer<std::string>;

#endif // QSETTINGSCONTAINER_H
