/***************************************************************************
 *   This file is part of the Lime Report project                          *
 *   Copyright (C) 2015 by Alexander Arin                                  *
 *   arin_a@bk.ru                                                          *
 *                                                                         *
 **                   GNU General Public License Usage                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 **                  GNU Lesser General Public License                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library.                                      *
 *   If not, see <http://www.gnu.org/licenses/>.                           *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ****************************************************************************/
#include <QDebug>
#include <QString>

#include "lrglobal.h"

namespace LimeReport {

QString extractClassName(QString className) {
  int startPos = className.lastIndexOf("::");
  if (startPos == -1)
    startPos = 0;
  else
    startPos += 2;
  return className.right(className.length() - startPos);
}

bool ReportSettings::suppressAbsentFieldsAndVarsWarnings() const { return m_suppressAbsentFieldsAndVarsWarnings; }

void ReportSettings::setSuppressAbsentFieldsAndVarsWarnings(bool suppressAbsentFieldsAndVarsWarnings) { m_suppressAbsentFieldsAndVarsWarnings = suppressAbsentFieldsAndVarsWarnings; }

QString escapeSimbols(const QString &value) {
  QString result = value;
  result.replace("\"", "\\\"");
  result.replace('\n', "\\n");
  return result;
}

QString replaceHTMLSymbols(const QString &value) {
  QString result = value;
  result.replace("<", "&lt;");
  result.replace(">", "&gt;");
  return result;
}

QVector<QString> normalizeCaptures(const QRegExp &reg) {
  QVector<QString> result;
  for (QString cap : reg.capturedTexts()) {
    if (!cap.isEmpty()) result.append(cap);
  }
  return result;
}

ReportError::ReportError(const QString &message) : std::runtime_error(message.toStdString()) {}

} // namespace LimeReport
