#include "manager.h"
#include "wintoastlib.h"

using namespace WinToastLib;

class CustomHandler : public IWinToastHandler {
 public:
  void toastActivated() { emit activated(notificationID); }
  void toastActivated(int) { emit activated(notificationID); }
  void toastFailed() {
    std::wcout << L"Error showing current toast" << std::endl;
  }
  void toastDismissed(WinToastDismissalReason) { emit dismissed(notificationID); }

  uint notificationID;

signals:
  void activated(uint id);
  void dismissed(uint id);
};

namespace {
bool isInitialized = false;
uint count = 0;

void init() {
  isInitialized = true;

  WinToast::instance()->setAppName(L"Spectral");
  WinToast::instance()->setAppUserModelId(
      WinToast::configureAUMI(L"Spectral", L"Spectral"));
  if (!WinToast::instance()->initialize())
    std::wcout << "Your system in not compatible with toast notifications\n";
}
}  // namespace

NotificationsManager::NotificationsManager(QObject *parent) : QObject(parent) {}

void NotificationsManager::postNotification(
    const QString &room_id, const QString &event_id, const QString &room_name,
    const QString &sender, const QString &text, const QImage &icon) {
  Q_UNUSED(room_id)
  Q_UNUSED(event_id)
  Q_UNUSED(icon)

  if (!isInitialized) init();

  auto templ = WinToastTemplate(WinToastTemplate::ImageAndText02);
  if (room_name != sender)
    templ.setTextField(
        QString("%1 - %2").arg(sender).arg(room_name).toStdWString(),
        WinToastTemplate::FirstLine);
  else
    templ.setTextField(QString("%1").arg(sender).toStdWString(),
                       WinToastTemplate::FirstLine);
  templ.setTextField(QString("%1").arg(text).toStdWString(),
                     WinToastTemplate::SecondLine);
  // TODO: implement room or user avatar
  // templ.setImagePath(L"C:/example.png");

  CustomHandler* customHandler = new CustomHandler();
  count++;
  customHandler->notificationID = count;
  notificationIds[count] = roomEventId{room_id, event_id};
  connect(customHandler, &CustomHandler::activated, this, [=](uint id) {
        this->actionInvoked(id, "");
  });
  connect(customHandler, &CustomHandler::dismissed, this, [=](uint id) {
        this->notificationClosed(id, 0);
  });

  WinToast::instance()->showToast(templ, customHandler);
}

void NotificationsManager::actionInvoked(uint id, QString action) {
    if (notificationIds.contains(id)) {
      roomEventId idEntry = notificationIds[id];
      emit notificationClicked(idEntry.roomId, idEntry.eventId);
    }
}

void NotificationsManager::notificationClosed(uint id, uint reason) {
    notificationIds.remove(id);
}
