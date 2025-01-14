﻿#include "SequenceTextFile.h"
#include "SystemApis.h"
#include "Variables.h"

using namespace Utau;

Q_CHARSET_DECLARE(SequenceTextFile)

// Project File Reader
SequenceTextFile::SequenceTextFile() : BaseFile(Qs::Default) {
    m_codec = defaultCodec;
    reset();
}

SequenceTextFile::SequenceTextFile(const QString &filename) : BaseFile(Qs::Default) {
    m_codec = defaultCodec;
    setFilename(filename);
}

SequenceTextFile::~SequenceTextFile() {
}

SequenceTextFile::SequenceTextFile(Qs::VariableSource source) : BaseFile(source) {
}

bool SequenceTextFile::loadCore(bool *valid) {
    QFile file(m_filename);
    QByteArray data;
    bool isUnicode = false;

    if (!file.open(QFile::ReadOnly | QIODevice::Text)) {
        if (*valid) {
            *valid = true;
        }
        return false;
    }

    data = file.readAll();
    file.close();

    // Detect Code
    QTextCodec *codec = GetUtfCodec(data);
    QTextStream in(&data);
    if (codec) {
        m_codec = codec;
    }
    in.setCodec(m_codec);

    // Read File
    QStringList currentSection;

    QString line;
    QString sectionName;
    QString sectionHead;

    QLinkNote curNote;
    int curLength;

    bool result = true;
    bool num = false;
    bool projectValid = true;

    bool findVersion, findSetting, findNote;

    findVersion = findSetting = findNote = false;
    curLength = 0;

    try {
        while (!in.atEnd()) {
            line = in.readLine();

            if (line.isEmpty() && !in.atEnd()) {
                continue;
            }

            // Continue to add until meet the start of section or end
            if (!line.startsWith(SECTION_BEGIN_MARK) && !in.atEnd()) {
                currentSection.push_back(line);
                continue;
            }

            // If meet end, append without continue
            if (!line.isEmpty() && in.atEnd()) {
                currentSection.push_back(line);
            }

            // Previous section is empty
            if (currentSection.size() <= 1) {

            } else {
                sectionHead = currentSection[0];
                // If Section Name is invalid
                if (!parseSectionName(sectionHead, sectionName)) {
                    currentSection.clear();
                    continue;
                }

                // If Section Name is a number
                num = false;
                sectionName.toInt(&num);

                if (num) {
                    // Parse Note
                    findNote = true;

                    curNote.clear();
                    if (parseSectionNote(currentSection, curNote)) {
                        // curNote.tick = curLength;

                        // Ignore note whose length is invalid
                        if (curNote.length > 0) {
                            m_sectionNotes.push_back(curNote);
                            curLength += curNote.length; // Add to main tick count
                        }

                    } else {
                        projectValid = false;
                    }
                } else if (sectionName == SECTION_NAME_VERSION) {
                    // Parse Version Sequence
                    findVersion = true;
                    if (!parseSectionVersion(currentSection, m_sectionVersion)) {
                        projectValid = false;
                    }
                    // Not Unicode
                    QString charset = m_sectionVersion.charset;
                    if (!isUnicode) {
                        if (!charset.isEmpty()) {
                            QTextCodec *newCodec =
                                QTextCodec::codecForName(m_sectionVersion.charset.toLatin1());
                            if (newCodec) {
                                in.setCodec(newCodec);
                            }
                        } else {
                        }
                    }
                } else if (sectionName == SECTION_NAME_SETTING) {
                    // Parse global settings
                    findSetting = true;
                    if (!parseSectionSettings(currentSection, m_sectionSettings)) {
                        projectValid = false;
                    }
                }
            }

            currentSection.clear();
            currentSection.push_back(line);
        }
        if (!(findVersion || findSetting || findNote)) {
            result = false;
            projectValid = false;
        }
    } catch (...) {
        result = false;
        projectValid = false;
    }

    if (valid) {
        *valid = projectValid;
    }

    return result;
}

bool SequenceTextFile::saveCore() {
    QFile file(m_filename);
    if (!file.open(QFile::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    QString charset = m_sectionVersion.charset;
    if (!charset.isEmpty()) {
        m_codec = QTextCodec::codecForName(charset.toLatin1()); // Write UTF-8
    }

    out.setCodec(m_codec);

    writeSectionVersion(out);  // Write Version
    writeSectionSettings(out); // Write Global Settings

    // Write Notes
    for (int i = 0; i < m_sectionNotes.size(); ++i) {
        writeSectionNote(i, m_sectionNotes.at(i), out);
    }

    writeSectionName(SECTION_NAME_TRACKEND, out); // Write End Sign
    file.close();

    return true;
}

void SequenceTextFile::resetCore() {
    m_sectionVersion.clear();
    m_sectionSettings.clear();
    m_sectionNotes.clear();
}

SectionVersion SequenceTextFile::sectionVersion() const {
    return m_sectionVersion;
}

SectionVersion &SequenceTextFile::sectionVersion() {
    return m_sectionVersion;
}

void SequenceTextFile::setSectionVersion(const SectionVersion &value) {
    m_sectionVersion = value;
}

QList<QLinkNote> SequenceTextFile::sectionNotes() const {
    return m_sectionNotes;
}

QList<QLinkNote> &SequenceTextFile::sectionNotes() {
    return m_sectionNotes;
}

void SequenceTextFile::setSectionNotes(const QList<QLinkNote> &value) {
    m_sectionNotes = value;
}

void SequenceTextFile::clearSectionNotes() {
    m_sectionNotes.clear();
}

void SequenceTextFile::appendSectionNote(const QLinkNote &oNote) {
    m_sectionNotes.push_back(oNote);
}

QLinkNote SequenceTextFile::popSectionNote() {
    QLinkNote aNote = m_sectionNotes.back();
    m_sectionNotes.pop_back();
    return aNote;
}

SectionSettings SequenceTextFile::sectionSettings() const {
    return m_sectionSettings;
}

SectionSettings &SequenceTextFile::sectionSettings() {
    return m_sectionSettings;
}

void SequenceTextFile::setSectionSettings(const SectionSettings &value) {
    m_sectionSettings = value;
}

bool SequenceTextFile::parseSectionName(const QString &oName, QString &oResult) {
    if (oName.startsWith(SECTION_BEGIN_MARK) && oName.endsWith(SECTION_END_MARK)) {
        oResult = oName.mid(SECTION_BEGIN_MARK.size(),
                            oName.size() - SECTION_BEGIN_MARK.size() - SECTION_END_MARK.size());
        return true;
    } else {
        return false;
    }
}

bool SequenceTextFile::parseSectionNote(const QStringList &sectionList, QLinkNote &note) {
    QStringList::size_type i;
    bool isValid = true;

    int eq;
    QString line;
    QString key, value;

    int valueInt;
    double valueDouble;
    bool isInt, isDouble;

    PBStrings mode2;
    QString strEnv;

    for (i = 0; i < sectionList.size(); ++i) {
        line = sectionList.at(i);
        eq = line.indexOf('=');
        if (eq <= 0) {
            continue;
        }

        key = line.left(eq);
        value = line.mid(eq + 1);

        valueInt = value.toInt(&isInt);
        valueDouble = value.toDouble(&isDouble);

        if (key == KEY_NAME_LYRIC) {
            note.lyric = value; // Lyric
        } else if (key == KEY_NAME_NOTE_NUM) {
            if (isInt) {
                note.noteNum = valueInt; // Note Num
            }
        } else if (key == KEY_NAME_LENGTH) {
            if (isInt) {
                note.length = valueInt; // Length
            }
        } else if (key == KEY_NAME_FLAGS) {
            note.flags = value; // Flags
        } else if (key == KEY_NAME_INTENSITY) {
            if (isDouble) {
                note.intensity = valueDouble; // Volume
            }
        } else if (key == KEY_NAME_MODULATION || key == KEY_NAME_MODURATION) {
            if (isDouble) {
                note.modulation = valueDouble; // Modulation
            }
        } else if (key == KEY_NAME_PRE_UTTERANCE) {
            if (isDouble) {
                note.preUttr = valueDouble; // PreUtterence
            }
        } else if (key == KEY_NAME_VOICE_OVERLAP) {
            if (isDouble) {
                note.overlap = valueDouble; // Voice Overlap
            }
        } else if (key == KEY_NAME_VELOCITY) {
            if (isDouble) {
                note.velocity = valueDouble; // Consonant Velocity
            }
        } else if (key == KEY_NAME_START_POINT) {
            if (isDouble) {
                note.stp = valueDouble; // StartPoint
            }
        } else if (key == KEY_NAME_TEMPO) {
            if (isDouble) {
                note.tempo = valueDouble; // Tempo
            }
        } else if (key == KEY_NAME_REGION_START) {
            note.region = value; // Start of region
        } else if (key == KEY_NAME_REGION_END) {
            note.regionEnd = value; // End of region
        } else if (key == KEY_NAME_PB_START) {
            if (isDouble) {
                note.pbstart = valueDouble; // Mode1 Start
            }
        } else if (key == KEY_NAME_PBS) {
            mode2.PBS = value; // Mode2 Start
        } else if (key == KEY_NAME_PBW) {
            mode2.PBW = value; // Mode2 Intervals
        } else if (key == KEY_NAME_PBY) {
            mode2.PBY = value; // Mode2 Offsets
        } else if (key == KEY_NAME_PBM) {
            mode2.PBM = value; // Mode2 Types
        } else if (key == KEY_NAME_PB_START) {
            if (isDouble) {
                note.pbstart = valueDouble; // Mode1 Start
            }
        } else if (key == KEY_NAME_PICHES || key == KEY_NAME_PITCHES ||
                   key == KEY_NAME_PITCH_BEND) {
            note.pitches = StringsToDoubles(value.split(COMMA)); // Mode1 Pitch
        } else if (key == KEY_NAME_VBR) {
            note.vibrato = StringsToDoubles(value.split(COMMA)); // Vibrato
        } else if (key == KEY_NAME_ENVELOPE) {
            strEnv = value; // Envelope
        } else if (!key.startsWith('@')) {
            note.customData.append(qMakePair(key, value)); // Custom Values
        }
    }
    note.Mode2Pitch = StringToPortamento(mode2); // Mode2 Pitch
    note.envelope = StringToEnvelope(strEnv);

    return isValid;
}

bool SequenceTextFile::parseSectionVersion(const QStringList &sectionList,
                                           SectionVersion &version) {
    int i;
    bool flag = false;

    int eq;
    QString line;
    QString key, value;

    for (i = 0; i < sectionList.size(); ++i) {
        line = sectionList.at(i);
        eq = line.indexOf('=');
        if (eq >= 0) {
            key = line.left(eq);
            value = line.mid(eq + 1);
            if (key == KEY_NAME_CHARSET) {
                m_sectionVersion.charset = value;
            }
            continue;
        }
        if (line.indexOf(UST_VERSION_PREFIX_NOSPACE) == 0) {
            version.version = line.mid(UST_VERSION_PREFIX_NOSPACE.size()).simplified();
            flag = true;
        }
    }

    return flag;
}

bool SequenceTextFile::parseSectionSettings(const QStringList &sectionList,
                                            SectionSettings &settings) {
    QStringList::size_type i;
    bool isValid = true;

    int eq;
    QString key, value;
    double num;
    bool isNum;

    for (i = 0; i < sectionList.size(); ++i) {
        eq = sectionList[i].indexOf("=");
        if (eq <= 0) {
            continue;
        }

        key = sectionList[i].left(eq);
        value = sectionList[i].mid(eq + 1);

        if (key == KEY_NAME_PROJECT_NAME) {
            settings.projectName = value; // Project Name
        } else if (key == KEY_NAME_OUTPUT_FILE) {
            settings.outputFileName = value; // Output File Name
        } else if (key == KEY_NAME_VOICE_DIR) {
            settings.voiceDirectory = fromUSTVoiceDir(value, AppPath); // Voice Directory
        } else if (key == KEY_NAME_CACHE_DIR) {
            settings.cacheDirectory = value; // Cache Directory
        } else if (key == KEY_NAME_TOOL1) {
            settings.wavtoolPath = fromUSTToolsDir(value, AppPath); // Wavtool
        } else if (key == KEY_NAME_TOOL2) {
            settings.resamplerPath = fromUSTToolsDir(value, AppPath); // Resampler
        } else if (key == KEY_NAME_MODE2) {
            if (value != "True") {
                isValid = false;
            }
            settings.isMode2 = true; // Mode2
        } else if (key == KEY_NAME_TEMPO) {
            num = value.toDouble(&isNum);
            if (!isNum) {
                isValid = false;
            } else {
                settings.globalTempo = num; // Global Tempo
            }
        } else if (key == KEY_NAME_FLAGS) {
            settings.globalFlags = value; // Flags
        }
    }
    return isValid;
}

void SequenceTextFile::writeSectionName(const int &name, QTextStream &out) {
    QString newName = QString::number(name);
    int nums = newName.size();
    for (int i = 0; i < 4 - nums; ++i) {
        newName.prepend("0");
    }
    writeSectionName(newName, out);
}

void SequenceTextFile::writeSectionName(const QString &name, QTextStream &out) {
    out << SECTION_BEGIN_MARK + name + SECTION_END_MARK << Qt::endl;
}

void SequenceTextFile::writeSectionNote(int num, const QLinkNote &note, QTextStream &out) {
    writeSectionName(num, out);

    // Items maybe not exist
    QString aVibrato = DoublesToStrings(note.vibrato).join(COMMA);
    QString aPitchBend = DoublesToStrings(note.pitches).join(COMMA);

    // Complex items
    PBStrings mode2;
    QString strEnvelope;

    mode2 = PortamentoToString(note.Mode2Pitch);
    strEnvelope = EnvelopeToString(note.envelope);

    // Items always exists
    out << KEY_NAME_LENGTH << "=" << note.length << Qt::endl;
    out << KEY_NAME_LYRIC << "=" << note.lyric << Qt::endl;
    out << KEY_NAME_NOTE_NUM << "=" << note.noteNum << Qt::endl;

    // Items can be omitted
    if (note.preUttr != NODEF_DOUBLE) {
        out << KEY_NAME_PRE_UTTERANCE << "=" << note.preUttr << Qt::endl;
    }
    if (note.overlap != NODEF_DOUBLE) {
        out << KEY_NAME_VOICE_OVERLAP << "=" << note.overlap << Qt::endl;
    }
    if (note.velocity != NODEF_DOUBLE) {
        out << KEY_NAME_VELOCITY << "=" << QString::number(note.velocity) << Qt::endl;
    }
    if (note.intensity != NODEF_DOUBLE) {
        out << KEY_NAME_INTENSITY << "=" << note.intensity << Qt::endl;
    }
    if (note.modulation != NODEF_DOUBLE) {
        out << KEY_NAME_MODULATION << "=" << note.modulation << Qt::endl;
    }
    if (note.stp != NODEF_DOUBLE) {
        out << KEY_NAME_START_POINT << "=" << note.stp << Qt::endl;
    }
    if (!note.flags.isEmpty()) {
        out << KEY_NAME_FLAGS << "=" << note.flags << Qt::endl;
    }

    // Items may not exist
    if (!note.pitches.isEmpty()) {
        out << KEY_NAME_PB_TYPE << "=" << VALUE_PITCH_TYPE << Qt::endl;
        out << KEY_NAME_PB_START << "=" << note.pbstart << Qt::endl;
        out << KEY_NAME_PITCH_BEND << "=" << aPitchBend << Qt::endl;
    }
    if (!note.Mode2Pitch.isEmpty()) {
        out << KEY_NAME_PBS << "=" << mode2.PBS << Qt::endl;
        out << KEY_NAME_PBW << "=" << mode2.PBW << Qt::endl;
        if (!mode2.PBY.isEmpty()) {
            out << KEY_NAME_PBY << "=" << mode2.PBY << Qt::endl;
        }
        if (!mode2.PBS.isEmpty()) {
            out << KEY_NAME_PBM << "=" << mode2.PBM << Qt::endl;
        }
    }
    if (!note.envelope.isEmpty()) {
        out << KEY_NAME_ENVELOPE << "=" << strEnvelope << Qt::endl;
    }
    if (!note.vibrato.isEmpty()) {
        out << KEY_NAME_VBR << "=" << aVibrato << Qt::endl;
    }
    if (note.tempo != NODEF_DOUBLE) {
        out << KEY_NAME_TEMPO << "=" << note.tempo << Qt::endl;
    }
    if (note.region != NODEF_STRING) {
        out << KEY_NAME_REGION_START << "=" << note.region << Qt::endl;
    }
    if (note.regionEnd != NODEF_STRING) {
        out << KEY_NAME_REGION_END << "=" << note.regionEnd << Qt::endl;
    }

    // Custom Values
    for (int i = 0; i < note.customData.size(); ++i) {
        out << note.customData[i].first << "=" << note.customData[i].second << Qt::endl;
    }
}

void SequenceTextFile::writeSectionVersion(QTextStream &out) {
    writeSectionName(SECTION_NAME_VERSION, out);

    out << UST_VERSION_PREFIX_NOSPACE << m_sectionVersion.version << Qt::endl;

    // UTF-8 UST File?
    QString charset = m_sectionVersion.charset;
    if (!charset.isEmpty()) {
        out << KEY_NAME_CHARSET << "=" << charset << Qt::endl;
    }
}

void SequenceTextFile::writeSectionSettings(QTextStream &oStream) {
    writeSectionName(SECTION_NAME_SETTING, oStream);

    oStream << KEY_NAME_TEMPO << "=" << m_sectionSettings.globalTempo << Qt::endl;
    oStream << KEY_NAME_TRACKS << "=" << VALUE_TRACKS_SINGLE << Qt::endl;
    oStream << KEY_NAME_PROJECT_NAME << "=" << m_sectionSettings.projectName << Qt::endl;
    oStream << KEY_NAME_VOICE_DIR << "=" << toUSTVoiceDir(m_sectionSettings.voiceDirectory, AppPath)
            << Qt::endl;
    oStream << KEY_NAME_OUTPUT_FILE << "=" << m_sectionSettings.outputFileName << Qt::endl;
    oStream << KEY_NAME_CACHE_DIR << "=" << m_sectionSettings.cacheDirectory << Qt::endl;
    oStream << KEY_NAME_TOOL1 << "=" << toUSTToolsDir(m_sectionSettings.wavtoolPath, AppPath)
            << Qt::endl;
    oStream << KEY_NAME_TOOL2 << "=" << toUSTToolsDir(m_sectionSettings.resamplerPath, AppPath)
            << Qt::endl;

    if (m_sectionSettings.isMode2) {
        oStream << KEY_NAME_MODE2 << "=" << VALUE_MODE2_ON << Qt::endl;
    }

    if (!m_sectionSettings.globalFlags.isEmpty()) {
        oStream << KEY_NAME_FLAGS << "=" << m_sectionSettings.globalFlags << Qt::endl;
    }
}
