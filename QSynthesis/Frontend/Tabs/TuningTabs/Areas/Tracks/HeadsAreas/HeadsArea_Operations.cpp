#include "../../../Modules/Elements/TrackHead.h"
#include "../../../Operations/TrackOperations/TrackModifyOperation.h"
#include "../../../TuningGroup.h"
#include "../HeadsArea.h"

void HeadsArea::saveOperation(TrackOperation *t) {
    if (t->differ()) {
        m_ptrs->tab->addHistory(t);
    } else {
        delete t;
        qDebug() << "[Track Operation] Addition Refused";
    }
}

bool HeadsArea::handleOperation(TrackOperation *t, bool undo) {
    TrackOperation::Type type = t->type();

    switch (type) {
    case TrackOperation::Modify: {
        TrackModifyOperation *tm = static_cast<TrackModifyOperation *>(t);
        int index = tm->index();
        switch (tm->change()) {
        case TrackModifyOperation::Name: {
            setTrackName(index, undo ? tm->origin() : tm->modified());
            break;
        }
        case TrackModifyOperation::Resampler: {
            setTrackResampler(index, undo ? tm->origin() : tm->modified());
            break;
        }
        case TrackModifyOperation::Flags: {
            setTrackFlags(index, undo ? tm->origin() : tm->modified());
            break;
        }
        case TrackModifyOperation::VoiceDatabase: {
            setTrackVoice(index, undo ? tm->origin() : tm->modified());
            break;
        }
        }
    }
    default:
        break;
    }
    return true;
}

void HeadsArea::handleNameChanged(QString orgVal, QString newVal) {
    TrackHead *head = qobject_cast<TrackHead *>(sender());
    int index = Tracks.indexOf(head);

    TrackModifyOperation *tm = new TrackModifyOperation(TrackModifyOperation::Name);
    tm->setIndex(index);
    tm->setOrigin(orgVal);
    tm->setModified(newVal);

    saveOperation(tm);

    emit nameChanged(index, newVal);
}

void HeadsArea::handleFlagsChanged(QString orgVal, QString newVal) {
    TrackHead *head = qobject_cast<TrackHead *>(sender());
    int index = Tracks.indexOf(head);

    TrackModifyOperation *tm = new TrackModifyOperation(TrackModifyOperation::Flags);
    tm->setIndex(index);
    tm->setOrigin(orgVal);
    tm->setModified(newVal);

    saveOperation(tm);

    emit flagsChanged(index, newVal);
}

void HeadsArea::handleVoiceChanged(QString orgVal, QString newVal) {
    TrackHead *head = qobject_cast<TrackHead *>(sender());
    int index = Tracks.indexOf(head);

    TrackModifyOperation *tm = new TrackModifyOperation(TrackModifyOperation::VoiceDatabase);
    tm->setIndex(index);
    tm->setOrigin(orgVal);
    tm->setModified(newVal);

    saveOperation(tm);

    emit voiceChanged(index, newVal);
}

void HeadsArea::handleResamplerChanged(QString orgVal, QString newVal) {
    TrackHead *head = qobject_cast<TrackHead *>(sender());
    int index = Tracks.indexOf(head);

    TrackModifyOperation *tm = new TrackModifyOperation(TrackModifyOperation::Resampler);
    tm->setIndex(index);
    tm->setOrigin(orgVal);
    tm->setModified(newVal);

    saveOperation(tm);

    emit resamplerChanged(index, newVal);
}