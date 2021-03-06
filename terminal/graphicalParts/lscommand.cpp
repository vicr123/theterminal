#include "lscommand.h"
#include "ui_lscommand.h"

lsCommand::lsCommand(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::lsCommand)
{
    ui->setupUi(this);
}

lsCommand::~lsCommand()
{
    model->deleteLater();
    delete ui;
}

void lsCommand::setDir(QDir dir, QStringList args) {
    model = new QFileSystemModel();

    QDir::Filters filters = model->filter();
    if (args.contains("-a")) {
        filters = QDir::AllEntries | QDir::Hidden;
    }
    model->setFilter(filters);

    model->setRootPath(dir.path());
    ui->filesListView->setModel(model);
    ui->filesListView->setRootIndex(model->index(dir.path()));

    ui->filesListView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->filesListView->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->filesListView->header()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->filesListView->header()->setSectionResizeMode(3, QHeaderView::Fixed);

    QMetaObject::Connection cn = connect(model, &QFileSystemModel::rowsInserted, [=] {
        tVariantAnimation* anim = new tVariantAnimation();
        anim->setStartValue(ui->filesListView->height());
        anim->setEndValue(model->rowCount(model->index(dir.path())) * (ui->filesListView->sizeHintForRow(0)) + ui->filesListView->header()->height());
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->setDuration(250);
        connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
            ui->filesListView->setFixedHeight(value.toInt());
        });
        connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
        anim->start();
    });
    connect(this, &lsCommand::destroyed, [=] {
        disconnect(cn);
    });
}

void lsCommand::on_filesListView_activated(const QModelIndex &index)
{
    QFileInfo info = model->fileInfo(index);
    if (info.isDir()) {
        emit executeCommands(QStringList() << "cd \"" + info.filePath() + "\"" << "ls");
        emit scrollToBottom();
    }
}
