#include "modflagicondelegate.h"
#include <QList>


ModInfo::EFlag ModFlagIconDelegate::m_ConflictFlags[4] = { ModInfo::FLAG_CONFLICT_MIXED
                                                         , ModInfo::FLAG_CONFLICT_OVERWRITE
                                                         , ModInfo::FLAG_CONFLICT_OVERWRITTEN
                                                         , ModInfo::FLAG_CONFLICT_REDUNDANT };

ModInfo::EFlag ModFlagIconDelegate::m_ArchiveLooseConflictFlags[2] = { ModInfo::FLAG_ARCHIVE_LOOSE_CONFLICT_OVERWRITE
                                                                     , ModInfo::FLAG_ARCHIVE_LOOSE_CONFLICT_OVERWRITTEN };

ModInfo::EFlag ModFlagIconDelegate::m_ArchiveConflictFlags[3] = { ModInfo::FLAG_ARCHIVE_CONFLICT_MIXED
                                                                , ModInfo::FLAG_ARCHIVE_CONFLICT_OVERWRITE
                                                                , ModInfo::FLAG_ARCHIVE_CONFLICT_OVERWRITTEN };

ModFlagIconDelegate::ModFlagIconDelegate(QObject *parent, int logicalIndex, int compactSize)
  : IconDelegate(parent)
  , m_LogicalIndex(logicalIndex)
  , m_CompactSize(compactSize)
  , m_Compact(false)
{
}

void ModFlagIconDelegate::columnResized(int logicalIndex, int, int newSize)
{
  if (logicalIndex == m_LogicalIndex) {
    m_Compact = newSize < m_CompactSize;
  }
}

QList<QString> ModFlagIconDelegate::getIcons(const QModelIndex &index) const {
  QList<QString> result;
  QVariant modid = index.data(Qt::UserRole + 1);
  if (modid.isValid()) {
    ModInfo::Ptr info = ModInfo::getByIndex(modid.toInt());
    std::vector<ModInfo::EFlag> flags = info->getFlags();

    // insert conflict icons to provide nicer alignment
    { // insert loose file conflicts first
      auto iter = std::find_first_of(flags.begin(), flags.end(),
                                     m_ConflictFlags, m_ConflictFlags + 4);
      if (iter != flags.end()) {
        result.append(getFlagIcon(*iter));
        flags.erase(iter);
      } else if (!m_Compact) {
        result.append(QString());
      }
    }

    { // insert loose vs archive overwrite second
      auto iter = std::find(flags.begin(), flags.end(),
        ModInfo::FLAG_ARCHIVE_LOOSE_CONFLICT_OVERWRITE);
      if (iter != flags.end()) {
        result.append(getFlagIcon(*iter));
        flags.erase(iter);
      } else if (!m_Compact) {
        result.append(QString());
      }
    }

    { // insert loose vs archive overwritten third
      auto iter = std::find_first_of(flags.begin(), flags.end(),
        m_ArchiveLooseConflictFlags + 1, m_ArchiveLooseConflictFlags + 2);
      if (iter != flags.end()) {
        result.append(getFlagIcon(*iter));
        flags.erase(iter);
      } else if (!m_Compact) {
        result.append(QString());
      }
    }

    { // insert archive conflicts last
      auto iter = std::find_first_of(flags.begin(), flags.end(),
        m_ArchiveConflictFlags, m_ArchiveConflictFlags + 3);
      if (iter != flags.end()) {
        result.append(getFlagIcon(*iter));
        flags.erase(iter);
      } else if (!m_Compact) {
        result.append(QString());
      }
    }

    for (auto iter = flags.begin(); iter != flags.end(); ++iter) {
      auto iconPath = getFlagIcon(*iter);
      if (!iconPath.isEmpty())
        result.append(iconPath);
    }
  }
  return result;
}

QString ModFlagIconDelegate::getFlagIcon(ModInfo::EFlag flag) const
{
  switch (flag) {
    case ModInfo::FLAG_BACKUP: return ":/MO/gui/emblem_backup";
    case ModInfo::FLAG_INVALID: return ":/MO/gui/problem";
    case ModInfo::FLAG_NOTENDORSED: return ":/MO/gui/emblem_notendorsed";
    case ModInfo::FLAG_NOTES: return ":/MO/gui/emblem_notes";
    case ModInfo::FLAG_CONFLICT_MIXED: return ":/MO/gui/emblem_conflict_mixed";
    case ModInfo::FLAG_CONFLICT_OVERWRITE: return ":/MO/gui/emblem_conflict_overwrite";
    case ModInfo::FLAG_CONFLICT_OVERWRITTEN: return ":/MO/gui/emblem_conflict_overwritten";
    case ModInfo::FLAG_CONFLICT_REDUNDANT: return ":MO/gui/emblem_conflict_redundant";
    case ModInfo::FLAG_ARCHIVE_LOOSE_CONFLICT_OVERWRITE: return ":/MO/gui/archive_loose_conflict_overwrite";
    case ModInfo::FLAG_ARCHIVE_LOOSE_CONFLICT_OVERWRITTEN: return ":/MO/gui/archive_loose_conflict_overwritten";
    case ModInfo::FLAG_ARCHIVE_CONFLICT_MIXED: return ":/MO/gui/archive_conflict_mixed";
    case ModInfo::FLAG_ARCHIVE_CONFLICT_OVERWRITE: return ":/MO/gui/archive_conflict_winner";
    case ModInfo::FLAG_ARCHIVE_CONFLICT_OVERWRITTEN: return ":/MO/gui/archive_conflict_loser";
    case ModInfo::FLAG_ALTERNATE_GAME: return ":MO/gui/alternate_game";
    case ModInfo::FLAG_FOREIGN: return QString();
    case ModInfo::FLAG_SEPARATOR: return QString();
    case ModInfo::FLAG_OVERWRITE: return QString();
    case ModInfo::FLAG_PLUGIN_SELECTED: return QString();
    default: 
      qWarning("ModInfo flag %d has no defined icon", flag); 
      return QString();
  }
}

size_t ModFlagIconDelegate::getNumIcons(const QModelIndex &index) const
{
  unsigned int modIdx = index.data(Qt::UserRole + 1).toInt();
  if (modIdx < ModInfo::getNumMods()) {
    ModInfo::Ptr info = ModInfo::getByIndex(modIdx);
    std::vector<ModInfo::EFlag> flags = info->getFlags();
    size_t count = flags.size();
    if (std::find_first_of(flags.begin(), flags.end(), m_ConflictFlags, m_ConflictFlags + 4) == flags.end()) {
      ++count;
    }
    return count;
  } else {
    return 0;
  }
}


QSize ModFlagIconDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &modelIndex) const
{
  size_t count = getNumIcons(modelIndex);
  unsigned int index = modelIndex.data(Qt::UserRole + 1).toInt();
  QSize result;
  if (index < ModInfo::getNumMods()) {
    result = QSize(static_cast<int>(count) * 40, 20);
  } else {
    result = QSize(1, 20);
  }
  if (option.rect.width() > 0) {
    result.setWidth(std::min(option.rect.width(), result.width()));
  }
  return result;
}

