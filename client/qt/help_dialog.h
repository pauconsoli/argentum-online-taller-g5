#ifndef HELP_DIALOG_H
#define HELP_DIALOG_H

#include <QDialog>

class QTextBrowser;
class QPushButton;

class HelpDialog: public QDialog {
    Q_OBJECT

 public:
    explicit HelpDialog(QWidget* parent = nullptr);
    ~HelpDialog() override = default;

 private:
    QTextBrowser* content;
    QPushButton* close_button;
};

#endif
