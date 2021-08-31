#include "../TuningTab.h"
#include "Areas/Editor/NotesArea.h"
#include "Dialogs/InputDialog.h"
#include "Dialogs/InsertLyricsDialog.h"
#include "Dialogs/ProjectSettingsDialog.h"
#include "Graphics/GraphicsNote.h"
#include "Operations/ProjectOperation.h"
#include "ProjectInfoHandler.h"
#include "TuningGroup.h"
#include "mainwindow.h"

void TuningTab::showLyricInsertion() {
    if (isPlaying()) {
        return;
    }
    QList<QLinkNote> notes = m_ptrs->notesArea->selectedNotes();
    QStringList lyrics;

    for (int i = 0; i < notes.size(); ++i) {
        QString lrc = notes[i].lyric;
        lyrics.append(lrc);
    }

    InsertLyricsDialog *dlg = new InsertLyricsDialog(lyrics);
    int code = -1;

    code = dlg->exec();
    bool ignoreRest = dlg->ignoreRest();
    bool replaceLyrics = dlg->replaceLyrics();

    dlg->deleteLater();

    if (code != 1) {
        return;
    }

    if (replaceLyrics) {
        m_ptrs->notesArea->replaceSelectedLyrics(lyrics, ignoreRest);
    } else {
        m_ptrs->notesArea->insertLyrics(lyrics);
    }
    change();
}

void TuningTab::showTempoEdit(bool remove) {
    if (isPlaying()) {
        return;
    }

    QList<NoteProperties> props = m_ptrs->notesArea->selectedProperties();

    double tempo;
    QString title, caption;
    caption = tr("Beats per minute (10~512)...");

    if (props.isEmpty()) {
        tempo = m_ptrs->notesArea->globalTempo();
        title = tr("Set Global Tempo");
    } else if (!remove) {
        tempo = props.front().tempo.toDouble();
        title = tr("Set Tempo");
    } else {
        m_ptrs->notesArea->modifySelectedTempo(-1);
        return;
    }

    QDoubleValidator *validator = new QDoubleValidator(10, 512, 3, this);
    validator->setNotation(QDoubleValidator::StandardNotation);

    InputDialog *dlg = new InputDialog(title, caption, tempo, validator);
    int code = dlg->exec();
    dlg->deleteLater();
    if (code != 1) {
        return;
    }

    m_ptrs->notesArea->modifySelectedTempo(tempo);
    change();
}

void TuningTab::showProjectCharset() {
    if (isPlaying()) {
        return;
    }
}

void TuningTab::insertRest() {
    if (isPlaying()) {
        return;
    }
    m_ptrs->notesArea->insertRest();
    updateMenuCore();
}

void TuningTab::removeRest() {
    if (isPlaying()) {
        return;
    }
    double length = 0;
    QIntValidator *validator = new QIntValidator(0, INT_MAX, this);
    InputDialog *dlg =
        new InputDialog(tr("Remove Rests"), tr("Remove rests shorter than..."), length, validator);

    int code = dlg->exec();
    dlg->deleteLater();

    if (code != 1) {
        return;
    }

    m_ptrs->notesArea->removeRestLessThan(length);
    change();
}

void TuningTab::setP2P3Fade() {
    m_ptrs->notesArea->modifySelectedEnvelope(Qs::P2P3Fade);
}

void TuningTab::setP1P4Fade() {
    m_ptrs->notesArea->modifySelectedEnvelope(Qs::P1P4Fade);
}

void TuningTab::resetEnvelope() {
    m_ptrs->notesArea->modifySelectedEnvelope(Qs::DefaultEnvelope);
}

void TuningTab::transpose(int offset) {
    if (isPlaying()) {
        return;
    }
    if (offset == 0) {
        double dbOffset = offset;
        QIntValidator *validator = new QIntValidator(-84, 84, this);
        InputDialog *dlg =
            new InputDialog(tr("Transpose"), tr("Pitch Shift(semitones)"), dbOffset, validator);

        int code = dlg->exec();
        dlg->deleteLater();

        if (code != 1) {
            return;
        }

        offset = dbOffset;
    }
    m_ptrs->notesArea->shiftSelectedNotes(offset);
    change();
}

void TuningTab::modifyAlias() {
    QList<QLinkNote> notes = m_ptrs->notesArea->selectedNotes();
    if (notes.size() != 1) {
        return;
    }
    QString lrc = notes.front().lyric;
    if (isRestNoteLyric(lrc)) {
        return;
    }
    lrc = lrc.mid(0, 1);
    QStringList alias = oto->findAliasStartsWith(lrc);

    m_tempMenu->setTexts(alias);
    int index = m_tempMenu->start();
    if (index >= 0) {
        m_ptrs->notesArea->replaceSelectedLyrics({alias.at(index)}, true);
    }
}