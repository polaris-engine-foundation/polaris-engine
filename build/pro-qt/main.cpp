/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: nil; -*- */

/*
 * x-engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

#include "mainwindow.h"

extern "C" {
#include "xengine.h"
#include "glrender.h"
};

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Create the main window and run app.
    MainWindow w;
    w.show();
    int ret = a.exec();

    return ret;
}
