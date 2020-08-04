#include "remotemanagementtoppanel.h"
#include "utils.h"
#include "mainwindow.h"

#include <QParallelAnimationGroup>
#include <QDebug>

const int animationDuration = 300;

RemoteManagementTopPanel::RemoteManagementTopPanel(QWidget *parent) : RightPanel(parent)
{
    // 远程主界面
    m_remoteManagementPanel = new RemoteManagementPanel(this);
    connect(m_remoteManagementPanel, &RemoteManagementPanel::showSearchPanel, this, &RemoteManagementTopPanel::showSearchPanel);
    connect(m_remoteManagementPanel, &RemoteManagementPanel::showGroupPanel, this, &RemoteManagementTopPanel::showGroupPanel);
    connect(m_remoteManagementPanel, &RemoteManagementPanel::doConnectServer, this, &RemoteManagementTopPanel::doConnectServer, Qt::QueuedConnection);
    // 远程分组界面
    m_serverConfigGroupPanel = new ServerConfigGroupPanel(this);
    connect(m_serverConfigGroupPanel, &ServerConfigGroupPanel::showSearchPanel, this, &RemoteManagementTopPanel::showSearchPanel);
    connect(m_serverConfigGroupPanel, &ServerConfigGroupPanel::rebackPrevPanel, this, &RemoteManagementTopPanel::showPrevPanel);
    connect(m_serverConfigGroupPanel, &ServerConfigGroupPanel::doConnectServer, this, &RemoteManagementTopPanel::doConnectServer, Qt::QueuedConnection);
    // 远程搜索界面
    m_remoteManagementSearchPanel = new RemoteManagementSearchPanel(this);
    connect(m_remoteManagementSearchPanel, &RemoteManagementSearchPanel::showGroupPanel, this, &RemoteManagementTopPanel::showGroupPanel);
    connect(m_remoteManagementSearchPanel, &RemoteManagementSearchPanel::rebackPrevPanel, this, &RemoteManagementTopPanel::showPrevPanel);
    connect(m_remoteManagementSearchPanel, &RemoteManagementSearchPanel::doConnectServer, this, &RemoteManagementTopPanel::doConnectServer, Qt::QueuedConnection);
    // 界面先隐藏
    m_serverConfigGroupPanel->hide();
    m_remoteManagementSearchPanel->hide();
    m_remoteManagementPanel->hide();
}

void RemoteManagementTopPanel::show()
{
    RightPanel::show();
    m_remoteManagementPanel->resize(size());
    m_remoteManagementPanel->move(0, 0);
    // 每次显示前清空之前的记录
    m_remoteManagementPanel->clearListFocus();
    // 显示远程主界面
    m_remoteManagementPanel->show();
    // 其他界面影藏
    m_serverConfigGroupPanel->hide();
    m_remoteManagementSearchPanel->hide();
    // 刷新列表
    m_remoteManagementPanel->refreshPanel();
    // 记录界面状态
    m_remoteManagementPanel->m_isShow = true;
    m_serverConfigGroupPanel->m_isShow = false;
    m_remoteManagementSearchPanel->m_isShow = false;
    // 记录当前窗口
    m_currentPanelType = ServerConfigManager::PanelType_Manage;
    m_prevPanelStack.push_back(m_currentPanelType);
}

/*******************************************************************************
 1. @函数:    setFocusInPanel
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-23
 4. @说明:    键盘进入设置焦点
*******************************************************************************/
void RemoteManagementTopPanel::setFocusInPanel()
{
    m_remoteManagementPanel->setFocusInPanel();
}

/*******************************************************************************
 1. @函数:    showSearchPanel
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-08-04
 4. @说明:    显示搜索界面
 若当前界面为主界面 => 全局搜索
 若当前界面为分组界面 => 组内搜索
*******************************************************************************/
void RemoteManagementTopPanel::showSearchPanel(const QString &strFilter)
{
    // 记录搜索界面的搜索条件
    m_filter = strFilter;
    // 设置搜索界面大小
    m_remoteManagementSearchPanel->resize(size());
    // 显示搜索界面
    m_remoteManagementSearchPanel->show();

    // 动画效果
    QPropertyAnimation *animation;
    QPropertyAnimation *animation1 = new QPropertyAnimation(m_remoteManagementSearchPanel, "geometry");
    connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
    // 判读当前窗口是主界面还是分组界面
    // 刷新搜索界面的列表
    if (m_currentPanelType == ServerConfigManager::PanelType_Group) {
        // 组内搜索
        m_remoteManagementSearchPanel->refreshDataByGroupAndFilter(m_group, m_filter);

        // 动画效果的设置
        animation = new QPropertyAnimation(m_serverConfigGroupPanel, "geometry");
        connect(animation, &QPropertyAnimation::finished, m_serverConfigGroupPanel, &QWidget::hide);
        connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);

    } else if (m_currentPanelType == ServerConfigManager::PanelType_Manage) {
        // 全局搜索
        m_remoteManagementSearchPanel->refreshDataByFilter(m_filter);

        // 动画效果的设置
        animation = new QPropertyAnimation(m_remoteManagementPanel, "geometry");
        connect(animation, &QPropertyAnimation::finished, m_remoteManagementPanel, &QWidget::hide);
        connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);

    } else {
        qDebug() << "unknow current panel!";
    }
    // 执行动画
    panelRightToLeft(animation, animation1);

    // 搜索界面设置焦点
    m_remoteManagementSearchPanel->onFocusInBackButton();
    // 记录当前窗口为前一个窗口
    m_prevPanelStack.push_back(m_currentPanelType);
    // 修改当前窗口
    m_currentPanelType = ServerConfigManager::PanelType_Search;
    // 设置平面状态
    setPanelShowState(m_currentPanelType);
}

/*******************************************************************************
 1. @函数:    showGroupPanel
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-08-04
 4. @说明:    显示分组平面
*******************************************************************************/
void RemoteManagementTopPanel::showGroupPanel(const QString &strGroupName, bool isFocusOn)
{
    // 记录当前分组
    m_group = strGroupName;
    // 设置分组界面大小
    m_serverConfigGroupPanel->resize(size());
    // 刷新分组界面
    m_serverConfigGroupPanel->refreshData(strGroupName);
    // 显示分组界面
    m_serverConfigGroupPanel->show();

    // 动画效果
    QPropertyAnimation *animation;
    QPropertyAnimation *animation1 = new QPropertyAnimation(m_serverConfigGroupPanel, "geometry");
    connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
    if (m_currentPanelType == ServerConfigManager::PanelType_Search) {
        // 动画效果的设置
        animation = new QPropertyAnimation(m_remoteManagementSearchPanel, "geometry");
        connect(animation, &QPropertyAnimation::finished, m_remoteManagementSearchPanel, &QWidget::hide);
        connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);

    } else if (m_currentPanelType == ServerConfigManager::PanelType_Manage) {
        // 动画效果的设置
        animation = new QPropertyAnimation(m_remoteManagementPanel, "geometry");
        connect(animation, &QPropertyAnimation::finished, m_remoteManagementPanel, &QWidget::hide);
        connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
    } else {
        qDebug() << "unknow current panel!";
    }
    // 执行动画
    panelRightToLeft(animation, animation1);


    // 分组界面设置焦点
    if (isFocusOn) {
        // 1）不是鼠标点击，焦点落在返回键上
        // 2）是鼠标点击，但是焦点在组上，焦点也要落在返回键上
        m_serverConfigGroupPanel->onFocusInBackButton();
    } else {
        // 是鼠标点击，当前项没有焦点
        Utils::getMainWindow(this)->focusCurrentPage();
        qDebug() << "show group but not focus in group";
    }

    // 记录当前窗口为前一个窗口
    m_prevPanelStack.push_back(m_currentPanelType);
    // 修改当前窗口
    m_currentPanelType = ServerConfigManager::PanelType_Group;
    // 设置平面状态
    setPanelShowState(m_currentPanelType);
}

/*******************************************************************************
 1. @函数:    showPrevPanel
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-08-04
 4. @说明:    显示前一个界面
*******************************************************************************/
void RemoteManagementTopPanel::showPrevPanel()
{
    // 栈为空
    if (m_prevPanelStack.isEmpty()) {
        // 返回
        qDebug() << "stack is empty!";
        return;
    }
    // 获取前一个界面的类型，此界面为现在要显示的界面
    ServerConfigManager::PanelType prevType = m_prevPanelStack.pop();
    // 动画效果 要隐藏的界面
    QPropertyAnimation *animation;
    if (m_currentPanelType == ServerConfigManager::PanelType_Search) {
        // 动画效果的设置
        animation = new QPropertyAnimation(m_remoteManagementSearchPanel, "geometry");
        connect(animation, &QPropertyAnimation::finished, m_remoteManagementSearchPanel, &QWidget::hide);
        connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
    } else if (m_currentPanelType == ServerConfigManager::PanelType_Group) {
        // 动画效果的设置
        animation = new QPropertyAnimation(m_serverConfigGroupPanel, "geometry");
        connect(animation, &QPropertyAnimation::finished, m_serverConfigGroupPanel, &QWidget::hide);
        connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
    } else {
        qDebug() << "unknow panel to hide!" << m_currentPanelType;
    }

    // 动画效果 要显示的界面
    QPropertyAnimation *animation1;
    switch (prevType) {
    case ServerConfigManager::PanelType_Manage:
        // 刷新主界面
        m_remoteManagementPanel->refreshPanel();
        // 显示主界面
        m_remoteManagementPanel->show();
        // 动画效果的设置
        animation1 = new QPropertyAnimation(m_remoteManagementPanel, "geometry");
        connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
        break;
    case ServerConfigManager::PanelType_Group:
        // 刷新分组列表
        m_serverConfigGroupPanel->refreshData(m_group);
        // 显示分组
        m_serverConfigGroupPanel->show();
        // 动画效果的设置
        animation1 = new QPropertyAnimation(m_serverConfigGroupPanel, "geometry");
        connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
        break;
    case ServerConfigManager::PanelType_Search:
        // 刷新列表 => 搜索框能被返回，只能是全局搜索
        m_remoteManagementSearchPanel->refreshDataByFilter(m_filter);
        // 显示搜索框
        m_remoteManagementSearchPanel->show();
        // 动画效果的设置
        animation1 = new QPropertyAnimation(m_remoteManagementSearchPanel, "geometry");
        connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
        break;
    default:
        qDebug() << "unknow current panel to show!" << prevType;
        break;
    }
    // 执行动画
    panelLeftToRight(animation, animation1);

    // 焦点返回
    if (m_currentPanelType  == ServerConfigManager::PanelType_Search) {
        // 搜索返回
        if (prevType == ServerConfigManager::PanelType_Manage) {
            // 返回主界面
            m_remoteManagementPanel->setFocusInPanel();
        } else {
            // 返回分组
            m_serverConfigGroupPanel->setFocusBack();
        }
    } else {
        // 分组返回
        if (prevType == ServerConfigManager::PanelType_Manage) {
            // 返回主界面
            m_remoteManagementPanel->setFocusBack(m_group);
        } else {
            // 返回搜索界面
            m_remoteManagementSearchPanel->setFocusBack(m_group);
        }
    }

    // 修改当前窗口
    m_currentPanelType = prevType;
    // 设置平面状态
    setPanelShowState(m_currentPanelType);
}

/*******************************************************************************
 1. @函数:    setPanelShowState
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-08-04
 4. @说明:    设置平面状态
*******************************************************************************/
void RemoteManagementTopPanel::setPanelShowState(ServerConfigManager::PanelType panelType)
{
    // 设置平面状态
    m_remoteManagementPanel->m_isShow = false;
    m_serverConfigGroupPanel->m_isShow = false;
    m_remoteManagementSearchPanel->m_isShow = false;

    switch (panelType) {
    case ServerConfigManager::PanelType_Manage:
        m_remoteManagementPanel->m_isShow = true;
        break;
    case ServerConfigManager::PanelType_Group:
        m_serverConfigGroupPanel->m_isShow = true;
        break;
    case ServerConfigManager::PanelType_Search:
        m_remoteManagementSearchPanel->m_isShow = true;
        break;
    default:
        qDebug() << "panelType is wrong!";
        break;
    }
}

void RemoteManagementTopPanel::panelLeftToRight(QPropertyAnimation *animation, QPropertyAnimation *animation1)
{
    animation->setDuration(animationDuration);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    QRect rect = geometry();
    animation->setStartValue(QRect(0, rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(rect.width(), rect.y(), rect.width(), rect.height()));

    animation1->setDuration(animationDuration);
    animation1->setEasingCurve(QEasingCurve::OutQuad);
    animation1->setStartValue(QRect(-rect.width(), rect.y(), rect.width(), rect.height()));
    animation1->setEndValue(QRect(0, rect.y(), rect.width(), rect.height()));
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(animation);
    group->addAnimation(animation1);
    // 已验证：这个设定，会释放group以及所有组内动画。
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void RemoteManagementTopPanel::panelRightToLeft(QPropertyAnimation *animation, QPropertyAnimation *animation1)
{
    animation->setDuration(animationDuration);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    QRect rect = geometry();
    animation->setStartValue(QRect(0, rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(-rect.width(), rect.y(), rect.width(), rect.height()));

    animation1->setDuration(animationDuration);
    animation1->setEasingCurve(QEasingCurve::OutQuad);
    animation1->setStartValue(QRect(rect.width(), rect.y(), rect.width(), rect.height()));
    animation1->setEndValue(QRect(0, rect.y(), rect.width(), rect.height()));
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(animation);
    group->addAnimation(animation1);
    // 已验证：这个设定，会释放group以及所有组内动画。
    group->start(QAbstractAnimation::DeleteWhenStopped);
}
