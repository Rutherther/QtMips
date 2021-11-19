﻿// SPDX-License-Identifier: GPL-2.0+
/*******************************************************************************
 * QtMips - MIPS 32-bit Architecture Subset Simulator
 *
 * Implemented to support following courses:
 *
 *   B35APO - Computer Architectures
 *   https://cw.fel.cvut.cz/wiki/courses/b35apo
 *
 *   B4M35PAP - Advanced Computer Architectures
 *   https://cw.fel.cvut.cz/wiki/courses/b4m35pap/start
 *
 * Copyright (c) 2017-2019 Karel Koci<cynerd@email.cz>
 * Copyright (c) 2019      Pavel Pisa <pisa@cmp.felk.cvut.cz>
 *
 * Faculty of Electrical Engineering (http://www.fel.cvut.cz)
 * Czech Technical University        (http://www.cvut.cz/)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 ******************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "assembler/simpleasm.h"
#include "cachedock.h"
#include "cop0dock.h"
#include "coreview.h"
#include "extprocess.h"
#include "gui/srceditor.h"
#include "lcddisplaydock.h"
#include "machine/machine.h"
#include "machine/machineconfig.h"
#include "memorydock.h"
#include "messagesdock.h"
#include "newdialog.h"
#include "peripheralsdock.h"
#include "programdock.h"
#include "registersdock.h"
#include "terminaldock.h"
#include "ui_MainWindow.h"

#include <QMainWindow>
#include <QPointer>
#include <QSettings>
#include <QTabWidget>

class MainWindow : public QMainWindow {
    Q_OBJECT

    friend class SimpleAsmWithEditorCheck;

public:
    MainWindow(OWNED QSettings *settings, QWidget *parent = nullptr);
    ~MainWindow() override;

    void start();
    void create_core(
        const machine::MachineConfig &config,
        bool load_executable = true,
        bool keep_memory = false);

    bool configured();

signals:
    void report_message(
        messagetype::Type type,
        QString file,
        int line,
        int column,
        QString text,
        QString hint);
    void clear_messages();

public slots:
    // Actions signals
    void new_machine();
    void machine_reload(
        bool force_memory_reset = false,
        bool force_elf_load = false);
    void print_action();
    void new_source();
    void open_source();
    void save_source();
    void save_source_as();
    void close_source();
    void close_source_check();
    void close_source_decided(int result);
    void example_source(const QString &source_file);
    void compile_source();
    void build_execute();
    void build_execute_no_check();
    void build_execute_with_save(bool cancel, QStringList tosavelist);
    void show_registers();
    void show_program();
    void show_memory();
    void show_cache_data();
    void show_cache_program();
    void show_peripherals();
    void show_terminal();
    void show_lcd_display();
    void show_cop0dock();
    void show_hide_coreview(bool show);
    void show_symbol_dialog();
    void show_messages();
    // Actions - help menu
    void about_program();
    void about_qt();
    // Actions - execution speed
    void set_speed();
    // Machine signals
    void machine_status(enum machine::Machine::Status st);
    void machine_exit();
    void machine_trap(machine::SimulatorException &e);
    void central_tab_changed(int index);
    void tab_widget_destroyed(QObject *obj);
    void view_mnemonics_registers(bool enable);
    void message_selected(
        messagetype::Type type,
        const QString &file,
        int line,
        int column,
        const QString &text,
        const QString &hint);
    void save_exit_or_ignore(bool cancel, const QStringList &tosavelist);

protected:
    void closeEvent(QCloseEvent *event) override;
    void setCurrentSrcEditor(SrcEditor *srceditor);

protected slots:
    void src_editor_save_to(const QString &filename);
    void build_execute_finished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    Ui::MainWindow *ui {};

    NewDialog *ndialog {};
    QTabWidget *central_window {};

    GraphicsView *coreview {};
    CoreViewScene *corescene;

    RegistersDock *registers {};
    ProgramDock *program {};
    MemoryDock *memory {};
    CacheDock *cache_program {}, *cache_data {};
    PeripheralsDock *peripherals {};
    TerminalDock *terminal {};
    LcdDisplayDock *lcd_display {};
    Cop0Dock *cop0dock {};
    MessagesDock *messages {};
    bool coreview_shown;
    SrcEditor *current_srceditor;

    QActionGroup *speed_group {};

    OWNED QSettings *settings;

    machine::Machine *machine; // Current simulated machine

    void show_dockwidget(
        QDockWidget *w,
        Qt::DockWidgetArea area = Qt::RightDockWidgetArea);
    void add_src_editor_to_tabs(SrcEditor *editor);
    void update_open_file_list();
    bool modified_file_list(QStringList &list, bool report_unnamed = false);
    SrcEditor *source_editor_for_file(const QString &filename, bool open);
    QPointer<ExtProcess> build_process;
    bool ignore_unsaved;
};

class SimpleAsmWithEditorCheck : public SimpleAsm {
    Q_OBJECT
    using Super = SimpleAsm;

public:
    SimpleAsmWithEditorCheck(MainWindow *a_mainwindow, QObject *parent = nullptr)
        : Super(parent)
        , mainwindow(a_mainwindow) {}
    bool process_file(const QString &filename, QString *error_ptr = nullptr)
        override;

protected:
    bool process_pragma(
        QStringList &operands,
        const QString &filename = "",
        int line_number = 0,
        QString *error_ptr = nullptr) override;

private:
    MainWindow *mainwindow;
};

#endif // MAINWINDOW_H
