#ifndef CALENDAR_ITEM_H
#define CALENDAR_ITEM_H

#include <QString>
#include <QDateTime>

namespace Calendar {
	class CalendarItem
	{
	public:
		CalendarItem(const QString &uid, const QDateTime &beginning, const QDateTime &ending) :
			m_uid(uid), m_beginning(beginning), m_ending(ending) {};

		// unicity
		const QString &uid() const { return m_uid; }

		const QString &title() const { return m_title; }
		void setTitle(const QString &value);

		const QDateTime &beginning() const { return m_beginning; }
		void setBeginning(const QDateTime &value);

		const QDateTime &ending() const { return m_ending; }
		void setEnding(const QDateTime &value);

		const QString &description() const { return m_description; }
		void setDescription(const QString &value);

	private:
		QString m_uid;
		QString m_title;
		QDateTime m_beginning;
		QDateTime m_ending;
		QString m_description;
	};
}

#endif
