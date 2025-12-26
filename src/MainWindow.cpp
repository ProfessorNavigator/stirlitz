/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <CreateProfileWindow.h>
#include <KeySetWindow.h>
#include <MainWindow.h>
#include <QClipboard>
#include <QDir>
#include <QFrame>
#include <QGridLayout>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include <SimpleFileEncryptionTab.h>
#include <fstream>

#ifndef __ANDROID__
#include <iostream>
#else
#include <android/log.h>
#define APPNAME "Stirlitz"
#endif

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
  home_p = std::filesystem::u8path(QDir::homePath().toStdString());
  home_p /= std::filesystem::u8path(".local");
  home_p /= std::filesystem::u8path("share");
  home_p /= "Stirlitz";

  spy = new Stirlitz;

  enterWindow();
}

MainWindow::~MainWindow()
{
  if(u_name)
    {
      gcry_free(u_name);
    }
  if(passwd)
    {
      gcry_free(passwd);
    }
  delete spy;
}

void
MainWindow::enterWindow()
{
  one_time_profile = false;
  if(u_name)
    {
      gcry_free(u_name);
      u_name = nullptr;
      u_name_sz = 0;
    }
  if(passwd)
    {
      gcry_free(passwd);
      passwd = nullptr;
      passwd_sz = 0;
    }

  key_pair.reset();
  other_key.reset();

  QWidget *central_widget = new QWidget;
  this->setCentralWidget(central_widget);

  QVBoxLayout *v_box = new QVBoxLayout;
  central_widget->setLayout(v_box);

  v_box->addStretch();

  QComboBox *profiles = new QComboBox;
  v_box->addWidget(profiles, 0, Qt::AlignCenter);

  if(std::filesystem::exists(home_p))
    {
      for(auto dir_it : std::filesystem::directory_iterator(home_p))
        {
          std::filesystem::path p = dir_it.path();
          if(std::filesystem::is_directory(p))
            {
              std::string item = p.filename().u8string();
              QString val(item.c_str());
              profiles->addItem(val);
            }
        }
    }

  QLineEdit *username = new QLineEdit;
  username->setPlaceholderText(tr("User name"));
  v_box->addWidget(username);

  QLineEdit *password = new QLineEdit;
  password->setPlaceholderText(tr("Password"));
  password->setEchoMode(QLineEdit::Password);
  v_box->addWidget(password);

  std::function<void()> ch_input = std::bind(
      &MainWindow::enterWindowCheckInput, this, profiles, username, password);

  connect(username, &QLineEdit::returnPressed, this, ch_input);

  connect(password, &QLineEdit::returnPressed, this, ch_input);

#ifndef __ANDROID__
  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);
#endif

  QPushButton *load = new QPushButton;
  load->setText(tr("Load profile"));
  connect(load, &QPushButton::clicked, this, ch_input);
#ifndef __ANDROID__
  h_box->addWidget(load, 0, Qt::AlignCenter);
#else
  v_box->addWidget(load, 0, Qt::AlignCenter);
#endif

  QPushButton *create = new QPushButton;
  create->setText(tr("Creat profile"));
  connect(create, &QPushButton::clicked, this,
          [this]
            {
              CreateProfileWindow *cpw = new CreateProfileWindow(this);
              cpw->creatWindow();
              connect(cpw, &CreateProfileWindow::signalProfile, this,
                      &MainWindow::mainWindow);
              cpw->show();
            });
#ifndef __ANDROID__
  h_box->addWidget(create, 0, Qt::AlignCenter);
#else
  v_box->addWidget(create, 0, Qt::AlignCenter);
#endif

  QPushButton *one_time = new QPushButton;
  one_time->setText(tr("One-time profile"));
  connect(one_time, &QPushButton::clicked, this,
          [this]
            {
              one_time_profile = true;
              mainWindow(std::string(), std::string(), std::string());
            });
  v_box->addWidget(one_time, 0, Qt::AlignCenter);

  QPushButton *local_file = new QPushButton;
  local_file->setText(tr("Local file encryption/decryption"));
  connect(local_file, &QPushButton::clicked, this,
          [this]
            {
              SimpleFileEncryptionTab *sfet
                  = new SimpleFileEncryptionTab(this, spy, true);
              sfet->creatWidget();
              sfet->show();
            });
  v_box->addWidget(local_file, 0, Qt::AlignCenter);

  QPushButton *close = new QPushButton;
  close->setText(tr("Close"));
  connect(close, &QPushButton::clicked, this, &MainWindow::close);
  v_box->addWidget(close, 0, Qt::AlignCenter);

  v_box->addStretch();
#ifndef __ANDROID__
  this->adjustSize();
#endif
}

void
MainWindow::enterWindowCheckInput(QComboBox *profiles, QLineEdit *username,
                                  QLineEdit *password)
{
  std::string prof_nm = profiles->currentText().toStdString();
  if(prof_nm.empty())
    {
      return void();
    }
  std::string unm = username->text().toStdString();
  if(unm.empty())
    {
      return void();
    }
  std::string passwd = password->text().toStdString();
  if(passwd.empty())
    {
      return void();
    }
  mainWindow(prof_nm, unm, passwd);
}

void
MainWindow::mainWindow(const std::string &profile, const std::string &username,
                       const std::string &password)
{
  std::filesystem::path prof_p;
  if(!one_time_profile)
    {
      u_name_sz = username.size();
      u_name = reinterpret_cast<char *>(gcry_malloc_secure(u_name_sz));
      for(size_t i = 0; i < u_name_sz; i++)
        {
          u_name[i] = username[i];
        }

      passwd_sz = password.size();
      passwd = reinterpret_cast<char *>(gcry_malloc_secure(passwd_sz));
      for(size_t i = 0; i < passwd_sz; i++)
        {
          passwd[i] = password[i];
        }

      prof_p = home_p / std::filesystem::u8path(profile)
               / std::filesystem::u8path("owk");
      std::filesystem::create_directories(prof_p.parent_path());
    }

  std::string error;
  if(std::filesystem::exists(prof_p))
    {
      std::fstream f;
      f.open(prof_p, std::ios_base::in | std::ios_base::binary);
      if(f.is_open())
        {
          std::string val;
          f.seekg(0, std::ios_base::end);
          val.resize(f.tellg());
          f.seekg(0, std::ios_base::beg);
          f.read(val.data(), val.size());
          f.close();

          try
            {
              val = spy->decryptData(username, password, val);
              key_pair = spy->sexpFromString(val);
            }
          catch(std::exception &er)
            {
              error = std::string(er.what());
            }
        }
    }
  else
    {
      try
        {
          key_pair = spy->generateKeyPair();
          if(!one_time_profile)
            {
              std::string val = spy->sexpToString(key_pair);
              val = spy->encryptData(username, password, val);

              std::fstream f;
              f.open(prof_p, std::ios_base::out | std::ios_base::binary);
              if(f.is_open())
                {
                  f.write(val.c_str(), val.size());
                  f.close();
                }
            }
        }
      catch(std::exception &er)
        {
          error = std::string(er.what());
        }
    }

  if(key_pair)
    {
      QWidget *central_widget = new QWidget;
      this->setCentralWidget(central_widget);

      QVBoxLayout *v_box = new QVBoxLayout;
      central_widget->setLayout(v_box);

      v_box->addWidget(ownKeyWidget(prof_p), 0);
      v_box->addWidget(otherKeyWidget(prof_p), 0);

      QHBoxLayout *h_box = new QHBoxLayout;
      v_box->addLayout(h_box, 0);
      h_box->addStretch();

      QPushButton *leav_profile = new QPushButton;
      leav_profile->setText(tr("Leave profile"));
      connect(leav_profile, &QPushButton::clicked, this,
              &MainWindow::enterWindow);
      h_box->addWidget(leav_profile, 0, Qt::AlignCenter);

      if(!one_time_profile)
        {
          QPushButton *remove_profile = new QPushButton;
          remove_profile->setText(tr("Remove profile"));
          connect(remove_profile, &QPushButton::clicked, this,
                  std::bind(&MainWindow::removeProfile, this,
                            prof_p.parent_path()));
          h_box->addWidget(remove_profile, 0, Qt::AlignCenter);
        }

      QPushButton *close = new QPushButton;
      close->setText(tr("Close application"));
      connect(close, &QPushButton::clicked, this, &MainWindow::close);
#ifndef __ANDROID__
      h_box->addWidget(close, 0, Qt::AlignCenter);
#else
      v_box->addWidget(close, 0, Qt::AlignRight);
#endif

      QTabWidget *work_mode_tabs = new QTabWidget;
      v_box->addWidget(work_mode_tabs, 1);

      text_tab = new TextTabWidget(nullptr, spy, key_pair, other_key);
      work_mode_tabs->addTab(text_tab, tr("Text"));

      file_tab = new FileTabWidget(nullptr, spy, key_pair, other_key);
      work_mode_tabs->addTab(file_tab, tr("File"));

      SimpleFileEncryptionTab *sfet
          = new SimpleFileEncryptionTab(nullptr, spy, false);
      work_mode_tabs->addTab(sfet, tr("Local file encryption"));
    }
  else
    {
#ifndef __ANDROID__
      std::cout << "MainWindow::mainWindow: " << error << std::endl;
#else
      __android_log_print(ANDROID_LOG_VERBOSE, APPNAME,
                          "MainWindow::mainWindow: %s", error.c_str());
#endif
      QMessageBox *msg = new QMessageBox(this);
      msg->setAttribute(Qt::WA_DeleteOnClose);
      msg->setWindowModality(Qt::WindowModality::WindowModal);

      msg->setIcon(QMessageBox::Icon::Critical);

      msg->setText(tr("Incorrect user name or password!"));

      QPushButton *close = msg->addButton(QMessageBox::StandardButton::Close);
      connect(close, &QPushButton::clicked, msg, &QMessageBox::close);

      msg->show();
    }
}

QWidget *
MainWindow::ownKeyWidget(const std::filesystem::path &prof_p)
{
  QFrame *result = new QFrame;
  result->setFrameShape(QFrame::Box);

  QVBoxLayout *v_box = new QVBoxLayout;
  result->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setText(tr("Own key:"));
  v_box->addWidget(lab, 1, Qt::AlignLeft);

  QScrollArea *scrl = new QScrollArea;
  v_box->addWidget(scrl, 0);

  std::string hex = spy->getPublicKeyString(key_pair);
  lab = new QLabel;
  lab->setText(hex.c_str());
  scrl->setWidget(lab);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box, 1);

  h_box->addStretch();

  QPushButton *copy = new QPushButton;
  copy->setText(tr("Copy"));
  connect(copy, &QPushButton::clicked, this,
          [scrl]
            {
              QClipboard *cbrd = QGuiApplication::clipboard();
              QLabel *lab = dynamic_cast<QLabel *>(scrl->widget());
              if(lab)
                {
                  cbrd->setText(lab->text());
                }
            });
  h_box->addWidget(copy, 0, Qt::AlignRight);

  QPushButton *reset = new QPushButton;
  reset->setText(tr("Reset"));
  connect(reset, &QPushButton::clicked, this,
          [this, scrl, prof_p]
            {
              resetOwnKey(prof_p, scrl);
            });
  h_box->addWidget(reset, 0, Qt::AlignCenter);

  return result;
}

void
MainWindow::resetOwnKey(const std::filesystem::path &prof_p, QScrollArea *scrl)
{
  QMessageBox *msg = new QMessageBox(this);
  msg->setAttribute(Qt::WA_DeleteOnClose);
  msg->setWindowModality(Qt::WindowModal);

  msg->setIcon(QMessageBox::Question);
  msg->setText(tr("Are you sure?"));

  msg->addButton(QMessageBox::Yes);
  msg->addButton(QMessageBox::No);

  connect(msg, &QMessageBox::buttonClicked, this,
          [msg, this, prof_p, scrl](QAbstractButton *button)
            {
              if(msg->buttonRole(button) == QMessageBox::YesRole)
                {
                  try
                    {
                      key_pair = spy->generateKeyPair();
                      if(!one_time_profile)
                        {
                          std::string val = spy->sexpToString(key_pair);
                          std::string username;
                          username.resize(u_name_sz);
                          for(size_t i = 0; i < u_name_sz; i++)
                            {
                              username[i] = u_name[i];
                            }
                          std::string password;
                          password.resize(passwd_sz);
                          for(size_t i = 0; i < passwd_sz; i++)
                            {
                              password[i] = passwd[i];
                            }
                          val = spy->encryptData(username, password, val);

                          std::filesystem::remove_all(prof_p);
                          std::fstream f;
                          f.open(prof_p,
                                 std::ios_base::out | std::ios_base::binary);
                          if(f.is_open())
                            {
                              f.write(val.c_str(), val.size());
                              f.close();
                            }
                        }
                    }
                  catch(std::exception &er)
                    {
#ifndef __ANDROID__
                      std::cout << "MainWindow::resetOwnKey: " << er.what()
                                << std::endl;
#else
                      __android_log_print(ANDROID_LOG_VERBOSE, APPNAME,
                                          "MainWindow::mainWindow: %s",
                                          er.what());
#endif
                    }
                  if(key_pair)
                    {
                      std::string hex = spy->getPublicKeyString(key_pair);
                      QLabel *lab = new QLabel;
                      lab->setText(hex.c_str());
                      scrl->setWidget(lab);
                    }
                  text_tab->resetKeys(key_pair, other_key);
                  file_tab->resetKeys(key_pair, other_key);
                }
            });

  msg->show();
}

QWidget *
MainWindow::otherKeyWidget(const std::filesystem::path &prof_p)
{
  std::filesystem::path other_key_p
      = prof_p.parent_path() / std::filesystem::u8path("otk");

  QFrame *result = new QFrame;
  result->setFrameShape(QFrame::Box);

  QVBoxLayout *v_box = new QVBoxLayout;
  result->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setText(tr("Other key:"));
  v_box->addWidget(lab, 1, Qt::AlignLeft);

  QScrollArea *scrl = new QScrollArea;
  v_box->addWidget(scrl, 0);

  lab = new QLabel;
  if(!one_time_profile)
    {
      std::fstream f;
      f.open(other_key_p, std::ios_base::in | std::ios_base::binary);
      if(f.is_open())
        {
          std::string val;
          f.seekg(0, std::ios_base::end);
          val.resize(f.tellg());
          f.seekg(0, std::ios_base::beg);
          f.read(val.data(), val.size());
          f.close();

          std::string username;
          username.resize(u_name_sz);
          for(size_t i = 0; i < u_name_sz; i++)
            {
              username[i] = u_name[i];
            }

          std::string password;
          password.resize(passwd_sz);
          for(size_t i = 0; i < passwd_sz; i++)
            {
              password[i] = passwd[i];
            }
          try
            {
              val = spy->decryptData(username, password, val);
              other_key = spy->sexpFromString(val);
              val = spy->getPublicKeyString(other_key);
            }
          catch(std::exception &er)
            {
#ifndef __ANDROID__
              std::cout << "MainWindow::otherKeyWidget: " << er.what()
                        << std::endl;
#else
              __android_log_print(ANDROID_LOG_VERBOSE, APPNAME,
                                  "MainWindow::otherKeyWidget: %s", er.what());
#endif
            }
          if(val.size() == 64)
            {
              lab->setText(val.c_str());
            }
        }
    }
  scrl->setWidget(lab);

  QPushButton *set = new QPushButton;
  set->setText(tr("Set"));
  connect(set, &QPushButton::clicked, this,
          [this, scrl, other_key_p]
            {
              KeySetWindow *ksw = new KeySetWindow(this);
              ksw->createWindow();
              connect(
                  ksw, &KeySetWindow::signalKey, this,
                  [this, scrl, other_key_p](const std::string &key)
                    {
                      std::string raw = spy->fromHex(key);
                      try
                        {
                          other_key = spy->generatePublicKeyExp(raw);
                        }
                      catch(std::exception &er)
                        {
#ifndef __ANDROID__
                          std::cout
                              << "MainWindow::otherKeyWidget: " << er.what()
                              << std::endl;
#else
                              __android_log_print(
                                  ANDROID_LOG_VERBOSE, APPNAME,
                                  "MainWindow::otherKeyWidget: %s", er.what());
#endif
                        }
                      if(other_key)
                        {
                          if(!one_time_profile)
                            {
                              std::string username;
                              username.resize(u_name_sz);
                              for(size_t i = 0; i < u_name_sz; i++)
                                {
                                  username[i] = u_name[i];
                                }

                              std::string password;
                              password.resize(passwd_sz);
                              for(size_t i = 0; i < passwd_sz; i++)
                                {
                                  password[i] = passwd[i];
                                }

                              std::string val = spy->sexpToString(other_key);
                              val = spy->encryptData(username, password, val);

                              std::filesystem::create_directories(
                                  other_key_p.parent_path());
                              std::filesystem::remove_all(other_key_p);
                              std::fstream f;
                              f.open(other_key_p, std::ios_base::out
                                                      | std::ios_base::binary);
                              if(f.is_open())
                                {
                                  f.write(val.c_str(), val.size());
                                  f.close();
                                }
                            }
                          std::string hex = spy->getPublicKeyString(other_key);

                          QLabel *lab = new QLabel;
                          lab->setText(hex.c_str());
                          scrl->setWidget(lab);
                        }
                      text_tab->resetKeys(key_pair, other_key);
                      file_tab->resetKeys(key_pair, other_key);
                    });
              ksw->show();
            });
  v_box->addWidget(set, 1, Qt::AlignRight);

  return result;
}

void
MainWindow::removeProfile(const std::filesystem::path &prof_dir_path)
{
  QMessageBox *msg = new QMessageBox(this);
  msg->setAttribute(Qt::WA_DeleteOnClose);
  msg->setWindowModality(Qt::WindowModal);

  msg->setIcon(QMessageBox::Question);
  msg->setText(tr("Are you sure?"));
  msg->addButton(QMessageBox::Yes);
  msg->addButton(QMessageBox::No);

  connect(msg, &QMessageBox::buttonClicked, this,
          [msg, this, prof_dir_path](QAbstractButton *button)
            {
              if(msg->buttonRole(button) == QMessageBox::YesRole)
                {
                  std::filesystem::remove_all(prof_dir_path);
                  if(std::filesystem::is_empty(prof_dir_path.parent_path()))
                    {
                      std::filesystem::remove_all(prof_dir_path.parent_path());
                    }
                  this->enterWindow();
                }
            });

  msg->show();
}
