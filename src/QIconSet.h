/*
 * Origin: https://github.com/SamirKharchi/qtex
 * Author: Samir Kharchi 2017
 * License: LGPL 3
 *
 * Assumes at least Qt 5.6
 */

#ifndef QICONSET_H
#define QICONSET_H

#include <QtCore/QObject>
#include <QtGui/QIcon>
#include <type_traits>

namespace qtex
{
    /*!
     * \class QIconSet
     * \brief Handles an icon set encoded in a single image
     *
     * Loads a single resource image containing a set of equally sized icons and allows
     * to retrieve individual icons out of the set in a 2D matrix-like fashion (i.e. by column and row)
     *
     * Technically this is a vectorized look-up table of icons, so it can be browsed by index or by row/column.
     *
     * This is for example useful to reduce the amount of resource files or store different icon states
     * in a single resource image. Also icons are accessible/modifiable at a central place and only
     * a single file path must be maintained.
     *
     * Also usually a UI interface graphics designer will have one (or several) icon set file(s) which let(s)
     * him check and review consistency/difference of style, color palette etc. so this is a straightforward
     * way to handle application icons dynamically.
     *
     * Todo: 
     * storage optimization. Currently there is no way to avoid "empty" icon areas which take up unnecessary memory.
     */
    class QIconSet : public QObject
    {
        Q_OBJECT

    public:
        /*!
         * Constructs an icon set out of a resource image file path
         * \param path The resource path of the icon set, e.g. ":/buttons/iconset.png"
         * \param colRow The (maximum) amount of icons contained in the resource image given as columns and rows
         * \param iconSize The size of each icon given in pixels
         * \param parent The parent QObject
         */
        template < typename T, typename U >
        QIconSet(T&& path, U&& colRow, U&& iconSize, QObject* parent = nullptr)
            : QObject       (parent)
            , m_iconset     (std::forward<T>(path))
            , m_matrixSize  (std::forward<U>(colRow))
            , m_iconSize    (std::forward<U>(iconSize))
        {
            setup();
        }

        /*!
        * Constructs an icon set out of a resource image file path which calculates the
        * uniform size of each icon automatically
        * \param path The resource path of the icon set, e.g. ":/buttons/iconset.png"
        * \param colRow The amount of icons contained in the resource image given as columns and rows
        * \param parent The parent QObject
        */
        template < typename T, typename U >
        QIconSet(T&& path, U&& colRow, QObject* parent = nullptr)
            : QIconSet(std::forward<T>(path), std::forward<U>(colRow), QPoint(), parent)
        {
        }

        /*!
        * Constructs an icon set out of a resource image file path assuming the icon set
        * to only have a single row.
        * \param path The resource path of the icon set, e.g. ":/buttons/iconset.png"
        * \param count The (maximum) amount of icons contained in the resource image
        * \param iconSize The size of each icon given in pixels
        * \param parent The parent QObject
        */
        template < typename T, typename U >
        QIconSet(T&& path, int count, U&& iconSize, QObject* parent = nullptr)
            : QIconSet(std::forward<T>(path), QPoint(count, 1), std::move(iconSize), parent)
        {
        }

        /*!
         * Constructs an icon set out of a resource image file path assuming the icon set
         * to only have a single row and the uniform size of each icon is automatically calculated.
         * \param path The resource path of the icon set, e.g. ":/buttons/iconset.png"
         * \param count The amount of icons contained in the resource image
         * \param parent The parent QObject
         */
        template < typename T >
        QIconSet(T&& path, int count, QObject* parent = nullptr)
            : QIconSet(std::forward<T>(path), QPoint(count, 1), QPoint(), parent)
        {
        }

        //! Move
        QIconSet(QIconSet&& src)
            : m_iconset     (std::move(src.m_iconset))
            , m_icons       (std::move(src.m_icons))
            , m_iconSize    (std::move(src.m_iconSize))
            , m_matrixSize  (std::move(src.m_matrixSize))
        {
        }

        /*!
         * Get an icon at the specified 1D position (zero-based index) in the icon set
         * \param index The column index of the icon
         * \remark The row index is assumed to be always 0
         * \returns The icon as a QIcon
         */
        const QIcon& getIcon(const int index) const
        {
            Q_ASSERT(isValid(index));
            if(index >= static_cast<int>(m_icons.size()))
            {
                return m_invalidIcon;
            }
            return m_icons[index];
        }

        /*!
         * Get an icon at the specified 1D position (zero-based index) in the icon set.
         * While the position is given by an enum!
         * \param index The column index of the icon
         * \remark The row index is assumed to be always 0
         * \returns The icon as a QIcon
         */
        template < typename ColType >
        const QIcon& getIcon(const ColType index) const
        {
            static_assert(std::is_enum<ColType>::value, "ColType must be of enum type");

            Q_ASSERT(isValid(underlying(index)));

            return getIcon(underlying(index));
        }

        /*!
         * Get an icon at the specified position in the icon set
         * \param col The column index of the icon
         * \param row The row index of the icon
         * \returns The icon as a QIcon
         */
        const QIcon& getIcon(const int col, const int row) const
        {
            Q_ASSERT(isValid(col, row));

            return m_icons[toIndex(col, row)];
        }

        /*!
         * Get an icon at the specified position in the icon set. While the position is given by enums!
         * \param col The column index of the icon
         * \param row The row index of the icon
         * \returns The icon as a QIcon
         */
        template < typename ColType, typename RowType >
        const QIcon& getIcon(const ColType col, const RowType row) const
        {
            static_assert(std::is_enum<ColType>::value, "ColType must be of enum type");
            static_assert(std::is_enum<RowType>::value, "RowType must be of enum type");

            Q_ASSERT(isValid(underlying(col), underlying(row)));

            return getIcon(underlying(col), underlying(row));
        }

        /*!
         * Check if there is an icon at the specified position
         * \param col The column index of the icon
         * \param row The row index of the icon
         * \returns True if an icon exists at the given position
         */
        bool isValid(const int col, const int row) const Q_DECL_NOEXCEPT
        {
            return toIndex(col, row) < static_cast<int>(m_icons.size());
        }

        /*!
         * Check if there is an icon at the specified 1D position
         * \param col The column index of the icon
         * \remark The row index is assumed to be 0
         * \returns True if an icon exists at the given position
         */
        bool isValid(const int index) const Q_DECL_NOEXCEPT
        {
            return index < static_cast<int>(m_icons.size());
        }

        /*!
         * Retrieves the icon size
         * \returns The icon size while the x-component is the width and y-component the height
         */
        const QPoint& getIconSize() const Q_DECL_NOEXCEPT
        {
            return m_iconSize;
        }

        /*!
         * Retrieves the icon's radius which is specifically useful for circular icons
         * \returns The icon's radius.
         */
        int getIconRadius() const Q_DECL_NOEXCEPT
        {
            return m_iconSize.x() * 0.5;
        }

    private:
        QPixmap            m_iconset;       //<! The original icon set
        std::vector<QIcon> m_icons;         //<! All extracted icons
        QPoint             m_matrixSize;    //<! The icon set matrix size
        QPoint             m_iconSize;      //<! The size of each icon
        QIcon              m_invalidIcon;   //<! Can be returned in case of an icon failure

        void setup()
        {
            if(m_iconset.isNull())
            {
                Q_ASSERT(false);
                return;
            }

            // If null we auto calculate the icon size out of the colRow infos
            if(iconSize.isNull())
            {
                m_iconSize.setX(m_iconset.width() / colRow.x());
                m_iconSize.setY(m_iconset.height() / colRow.y());
            }

            // Now retrieve all icons from the icon set.
            m_icons.reserve(m_matrixSize.x() * m_matrixSize.y());
            for(auto y = 0; y < m_matrixSize.y(); ++y)
            {
                auto pos_iy = m_iconSize.y() * y;
                for(auto x = 0; x < m_matrixSize.x(); ++x)
                {
                    const auto pos_ix = m_iconSize.x() * x;
                    auto       pmap   = m_iconset.copy(pos_ix, pos_iy, m_iconSize.x(), m_iconSize.y());

                    m_icons.emplace_back(std::move(pmap));
                }
            }
        }
        /*!
         * Converts a 2D index position (column/row) into a 1D index
         * \param col The column index of the icon
         * \param row The row index of the icon
         * \returns The 1D index
         */
        int toIndex(const int col, const int row) const Q_DECL_NOEXCEPT
        {
            return row * m_matrixSize.x() + col;
        }

        /*!
         * Converts a 1D index into a 2D index position (column/row)
         * \param index The 1D index of the icon
         * \returns The 2D index
         */
        QPoint fromIndex(const int index) const Q_DECL_NOEXCEPT
        {
            auto row = index / m_matrixSize.x();
            auto col = index - (row * m_matrixSize.x());

            return QPoint(col, row);
        }
    };
}    // namespace qtex


#endif    // QICONSET_H