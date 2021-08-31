#include "EditorForm.h"
#include "../../TuningTab.h"
#include "../Areas/Editor/NotesArea.h"
#include "../Interfaces/EditorInterface.h"
#include "../TuningGroup.h"

using namespace Qs::Panels;

EditorForm::EditorForm(TuningTab *parent) : BaseForm(parent) {
    // Pointer preset
    m_ptrs = parent->ptrs();

    // Add Title Bar
    setDefaultTitleBar();
    content = new EditorInterface(this);
    setWidget(content);

    btnNote = new SwitchButton(tr("Note"), titleBar());
    btnPitch = new SwitchButton(tr("Pitch"), titleBar());
    btnEnvelope = new SwitchButton(tr("Envelope"), titleBar());

    btnNote->setProperty("type", "titleBar");
    btnPitch->setProperty("type", "titleBar");
    btnEnvelope->setProperty("type", "titleBar");

    btnsGroup = new SwitchButtonGroup(this);
    btnsGroup->addButton(btnNote);
    btnsGroup->addButton(btnPitch);
    btnsGroup->addButton(btnEnvelope);

    btnPlays = new PlayButtonGroup(this);
    btnEases = new EnvButtonGroup(this);

    titleBar()->addItem(btnNote, false, false);
    titleBar()->addItem(btnPitch, false, false);
    titleBar()->addItem(btnEnvelope, false, false);

    titleBar()->addItem(btnPlays, false, true);
    titleBar()->addItem(btnEases, false, true);

    connect(btnsGroup, &SwitchButtonGroup::oneDoubleClicked, this, &BaseForm::reverseFold);
    connect(btnsGroup, &SwitchButtonGroup::switched, this, &EditorForm::handleSwitchChanged);

    connect(btnPlays, &PlayButtonGroup::play, this, &EditorForm::onPlay);
    connect(btnPlays, &PlayButtonGroup::stop, this, &EditorForm::onStop);
    connect(btnPlays, &PlayButtonGroup::start, this, &EditorForm::onStart);
    connect(btnPlays, &PlayButtonGroup::end, this, &EditorForm::onEnd);

    connect(btnEases, &EnvButtonGroup::p2p3, this, &EditorForm::onEnvP2P3);
    connect(btnEases, &EnvButtonGroup::p1p4, this, &EditorForm::onEnvP1P4);
    connect(btnEases, &EnvButtonGroup::reset, this, &EditorForm::onEnvReset);

    // Pointers
    m_ptrs->editorContent = content;

    // Set First Status
    btnNote->setChecked(true);
}

TuningGroup *EditorForm::ptrs() const {
    return m_ptrs;
}

Editor EditorForm::status() const {
    return Editor(btnsGroup->currentIndex());
}

void EditorForm::setStatus(Editor status) {
    if (status == this->status()) {
        return;
    }
    btnsGroup->setCurrentIndex(int(status));
    content->setVisionStatus(status);
}

void EditorForm::updatePlayStatus(bool playing) {
    btnPlays->btnPlay->setStatus(playing);
}

void EditorForm::handleSwitchChanged() {
    content->setVisionStatus(status());
}

void EditorForm::onPlay() {
    m_ptrs->tab->play();
}

void EditorForm::onStop() {
    m_ptrs->tab->stop();
}

void EditorForm::onStart() {
    m_ptrs->tab->moveToStart();
}

void EditorForm::onEnd() {
    m_ptrs->tab->moveToEnd();
}

void EditorForm::onEnvP2P3() {
    m_ptrs->notesArea->modifySelectedEnvelope(Qs::P2P3Fade);
}

void EditorForm::onEnvP1P4() {
    m_ptrs->notesArea->modifySelectedEnvelope(Qs::P1P4Fade);
}

void EditorForm::onEnvReset() {
    m_ptrs->notesArea->modifySelectedEnvelope(Qs::DefaultEnvelope);
}