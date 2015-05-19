// Copyright 2010-2012, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "renderer/unix/window_manager.h"

#include "testing/base/public/gunit.h"
#include "testing/base/public/gmock.h"
#include "renderer/unix/gtk_window_mock.h"
#include "renderer/unix/gtk_wrapper_mock.h"

using ::testing::Expectation;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::StrEq;
using ::testing::_;

namespace mozc {
namespace renderer {
namespace gtk {

namespace {
MATCHER_P(PointEq, expected_point, "The expected point does not match") {
  return (arg.x == expected_point.x) && (arg.y == expected_point.y);
}
}  // namespace

TEST(WindowManagerTest, InitializeTest) {
  GtkWindowMock *candidate_window_mock = new GtkWindowMock();
  GtkWindowMock *infolist_window_mock = new GtkWindowMock();
  GtkWrapperMock *gtk_mock = new GtkWrapperMock();

  Expectation candidate_show
      = EXPECT_CALL(*candidate_window_mock, ShowWindow());
  EXPECT_CALL(*candidate_window_mock, HideWindow()).After(candidate_show);
  EXPECT_CALL(*candidate_window_mock, Initialize()).After(candidate_show);

  Expectation infolist_show = EXPECT_CALL(*infolist_window_mock, ShowWindow());
  EXPECT_CALL(*infolist_window_mock, HideWindow()).After(candidate_show);
  EXPECT_CALL(*infolist_window_mock, Initialize()).After(candidate_show);

  WindowManager manager(candidate_window_mock, infolist_window_mock, gtk_mock);

  manager.Initialize();
}

TEST(WindowManagerTest, HideAllWindowsTest) {
  GtkWindowMock *candidate_window_mock = new GtkWindowMock();
  GtkWindowMock *infolist_window_mock = new GtkWindowMock();
  GtkWrapperMock *gtk_mock = new GtkWrapperMock();

  EXPECT_CALL(*candidate_window_mock, HideWindow());
  EXPECT_CALL(*infolist_window_mock, HideWindow());

  WindowManager manager(candidate_window_mock, infolist_window_mock, gtk_mock);

  manager.HideAllWindows();
}

TEST(WindowManagerTest, ShowAllWindowsTest) {
  GtkWindowMock *candidate_window_mock = new GtkWindowMock();
  GtkWindowMock *infolist_window_mock = new GtkWindowMock();
  GtkWrapperMock *gtk_mock = new GtkWrapperMock();

  EXPECT_CALL(*candidate_window_mock, ShowWindow());
  EXPECT_CALL(*infolist_window_mock, ShowWindow());

  WindowManager manager(candidate_window_mock, infolist_window_mock, gtk_mock);

  manager.ShowAllWindows();
}

TEST(WindowManagerTest, UpdateLayoutTest) {
  {
    SCOPED_TRACE("Empty candidates should hide window, and do nothing.");
    commands::RendererCommand command;

    GtkWindowMock *candidate_window_mock = new StrictMock<GtkWindowMock>();
    GtkWindowMock *infolist_window_mock = new StrictMock<GtkWindowMock>();
    GtkWrapperMock *gtk_mock = new StrictMock<GtkWrapperMock>();

    EXPECT_CALL(*candidate_window_mock, HideWindow());
    EXPECT_CALL(*infolist_window_mock, HideWindow());

    WindowManager manager(candidate_window_mock, infolist_window_mock,
                          gtk_mock);

    manager.UpdateLayout(command);
  }
  {
    // Other case of UpdateLayout test is break-downed as UpdateCandidateWindow
    // and UpdateInfolistWindow.
  }
}

TEST(WindowManagerTest, ActivateTest) {
  // TODO(nona): Implement activate.
}

TEST(WindowManagerTest, IsAvailable) {
  // TODO(nona): Implement IsAvailable.
}

TEST(WindowManagerTest, SetSendCommandInterfaceTest) {
  GtkWindowMock *candidate_window_mock = new GtkWindowMock();
  GtkWindowMock *infolist_window_mock = new GtkWindowMock();
  GtkWrapperMock *gtk_mock = new GtkWrapperMock();

  client::SendCommandInterface *send_command_interface
      = reinterpret_cast<client::SendCommandInterface *>(0x12345678);

  WindowManager manager(candidate_window_mock, infolist_window_mock, gtk_mock);
  manager.SetSendCommandInterface(send_command_interface);

  EXPECT_EQ(send_command_interface, manager.send_command_interface_);
}

TEST(WindowManagerTest, SetWindowPosTest) {
  GtkWindowMock *candidate_window_mock = new GtkWindowMock();
  GtkWindowMock *infolist_window_mock = new GtkWindowMock();
  GtkWrapperMock *gtk_mock = new GtkWrapperMock();

  const Point direction(10, 20);
  EXPECT_CALL(*candidate_window_mock, Move(PointEq(direction)));

  WindowManager manager(candidate_window_mock, infolist_window_mock, gtk_mock);
  manager.SetWindowPos(direction.x, direction.y);
}

TEST(WindowManagerTest, ShouldShowCandidateWindowTest) {
  {
    SCOPED_TRACE("If it is not visible, return false.");
    commands::RendererCommand command;
    command.set_visible(false);
    command.mutable_output()->mutable_candidates();

    WindowManager manager(NULL, NULL, NULL);
    EXPECT_FALSE(manager.ShouldShowCandidateWindow(command));
  }
  {
    SCOPED_TRACE("If there is no Candidates message, return false.");
    commands::RendererCommand command;
    command.set_visible(true);
    command.mutable_output();

    WindowManager manager(NULL, NULL, NULL);
    EXPECT_FALSE(manager.ShouldShowCandidateWindow(command));
  }
  {
    SCOPED_TRACE("If there are no candidates, return false.");
    commands::RendererCommand command;
    command.set_visible(true);
    command.mutable_output()->mutable_candidates();

    WindowManager manager(NULL, NULL, NULL);
    EXPECT_FALSE(manager.ShouldShowCandidateWindow(command));
  }
  {
    commands::RendererCommand command;
    command.set_visible(true);
    commands::Candidates *candidates
        = command.mutable_output()->mutable_candidates();
    candidates->add_candidate();

    WindowManager manager(NULL, NULL, NULL);
    EXPECT_TRUE(manager.ShouldShowCandidateWindow(command));
  }
}

TEST(WindowManagerTest, ShouldShowInfolistWindowTest) {
  {
    SCOPED_TRACE("If it is not visible, return false.");
    commands::RendererCommand command;
    command.set_visible(false);
    command.mutable_output()->mutable_candidates();

    WindowManager manager(NULL, NULL, NULL);
    EXPECT_FALSE(manager.ShouldShowInfolistWindow(command));
  }
  {
    SCOPED_TRACE("If there is no Candidates message, return false.");
    commands::RendererCommand command;
    command.set_visible(true);
    command.mutable_output();

    WindowManager manager(NULL, NULL, NULL);
    EXPECT_FALSE(manager.ShouldShowInfolistWindow(command));
  }
  {
    SCOPED_TRACE("If there are no candidates, return false.");
    commands::RendererCommand command;
    command.set_visible(true);
    command.mutable_output()->mutable_candidates();

    WindowManager manager(NULL, NULL, NULL);
    EXPECT_FALSE(manager.ShouldShowInfolistWindow(command));
  }
  {
    SCOPED_TRACE("If there is no usages message, return false");
    commands::RendererCommand command;
    command.set_visible(true);
    commands::Candidates *candidates
        = command.mutable_output()->mutable_candidates();
    candidates->add_candidate();

    WindowManager manager(NULL, NULL, NULL);
    EXPECT_FALSE(manager.ShouldShowInfolistWindow(command));
  }
  {
    SCOPED_TRACE("If there is no Information List message, return false");
    commands::RendererCommand command;
    command.set_visible(true);
    commands::Candidates *candidates
        = command.mutable_output()->mutable_candidates();
    candidates->add_candidate();

    WindowManager manager(NULL, NULL, NULL);
    EXPECT_FALSE(manager.ShouldShowInfolistWindow(command));
  }
  {
    SCOPED_TRACE("If there are no information, return false");
    commands::RendererCommand command;
    command.set_visible(true);
    commands::Candidates *candidates
        = command.mutable_output()->mutable_candidates();
    candidates->add_candidate();
    commands::InformationList *usage = candidates->mutable_usages();
    usage->add_information();

    WindowManager manager(NULL, NULL, NULL);
    EXPECT_FALSE(manager.ShouldShowInfolistWindow(command));
  }
  {
    SCOPED_TRACE("If focused index is out of range, return false");
    commands::RendererCommand command;
    command.set_visible(true);
    commands::Candidates *candidates
        = command.mutable_output()->mutable_candidates();
    candidates->add_candidate();
    commands::InformationList *usage = candidates->mutable_usages();
    usage->add_information();

    candidates->set_focused_index(3);

    WindowManager manager(NULL, NULL, NULL);
    EXPECT_FALSE(manager.ShouldShowInfolistWindow(command));
  }
  {
    SCOPED_TRACE("If focused candidate has no information, return false");
    commands::RendererCommand command;
    command.set_visible(true);
    commands::Candidates *candidates
        = command.mutable_output()->mutable_candidates();
    commands::Candidates_Candidate *candidate = candidates->add_candidate();
    commands::InformationList *usage = candidates->mutable_usages();
    usage->add_information();

    candidates->set_focused_index(0);
    candidate->set_index(0);

    WindowManager manager(NULL, NULL, NULL);
    EXPECT_FALSE(manager.ShouldShowInfolistWindow(command));
  }
  {
    commands::RendererCommand command;
    command.set_visible(true);
    commands::Candidates *candidates
        = command.mutable_output()->mutable_candidates();
    commands::Candidates_Candidate *candidate = candidates->add_candidate();

    commands::InformationList *usage = candidates->mutable_usages();
    usage->add_information();

    candidates->set_focused_index(0);
    candidate->set_information_id(0);

    WindowManager manager(NULL, NULL, NULL);
    EXPECT_TRUE(manager.ShouldShowInfolistWindow(command));
  }
}

TEST(WindowManagerTest, UpdateCandidateWindowTest) {
  // TODO(nona): Cleanup redundant codes.
  {
    SCOPED_TRACE("Use caret location");
    GtkWindowMock *candidate_window_mock = new GtkWindowMock();
    GtkWindowMock *infolist_window_mock = new GtkWindowMock();
    GtkWrapperMock *gtk_mock = new GtkWrapperMock();

    commands::RendererCommand command;
    command.set_visible(true);
    commands::Candidates *candidates
        = command.mutable_output()->mutable_candidates();
    candidates->set_focused_index(0);

    commands::Candidates_Candidate *candidate = candidates->add_candidate();
    candidate->set_information_id(0);

    commands::InformationList *usage = candidates->mutable_usages();
    usage->add_information();

    const Rect client_cord_rect(10, 20, 30, 40);
    const Point window_position(15, 25);
    const Size window_size(35, 45);

    candidates->set_window_location(commands::Candidates::CARET);
    const Rect caret_rect(16, 26, 2, 13);
    commands::Rectangle *rectangle = candidates->mutable_caret_rectangle();
    rectangle->set_x(caret_rect.Left());
    rectangle->set_y(caret_rect.Top());
    rectangle->set_width(caret_rect.Width());
    rectangle->set_height(caret_rect.Height());
    const Point expected_window_position(
        caret_rect.Left() - client_cord_rect.Left(),
        caret_rect.Top() + caret_rect.Height());

    EXPECT_CALL(*candidate_window_mock, Update(_))
        .WillOnce(Return(window_size));
    EXPECT_CALL(*candidate_window_mock, GetCandidateColumnInClientCord())
        .WillOnce(Return(client_cord_rect));
    EXPECT_CALL(*candidate_window_mock, GetWindowPos())
        .WillOnce(Return(window_position));
    EXPECT_CALL(*candidate_window_mock,
                Move(PointEq(expected_window_position)));
    EXPECT_CALL(*candidate_window_mock, ShowWindow());

    GtkWidget *toplevel_widget = reinterpret_cast<GtkWidget*>(0x12345678);
    GdkScreen *toplevel_screen = reinterpret_cast<GdkScreen*>(0x87654321);
    const Rect screen_rect(0, 0, 4000, 4000);
    EXPECT_CALL(*gtk_mock, GtkWindowNew(GTK_WINDOW_TOPLEVEL))
        .WillOnce(Return(toplevel_widget));
    EXPECT_CALL(*gtk_mock, GtkWindowGetScreen(toplevel_widget))
        .WillOnce(Return(toplevel_screen));
    EXPECT_CALL(*gtk_mock, GdkScreenGetWidth(toplevel_screen))
        .WillOnce(Return(screen_rect.size.width));
    EXPECT_CALL(*gtk_mock, GdkScreenGetHeight(toplevel_screen))
        .WillOnce(Return(screen_rect.size.height));
    WindowManager manager(candidate_window_mock, infolist_window_mock,
                          gtk_mock);
    const Rect actual_rect = manager.UpdateCandidateWindow(command);

    EXPECT_EQ(actual_rect.origin.x, expected_window_position.x);
    EXPECT_EQ(actual_rect.origin.y, expected_window_position.y);
    EXPECT_EQ(actual_rect.size.width, window_size.width);
    EXPECT_EQ(actual_rect.size.height, window_size.height);
  }
  {
    SCOPED_TRACE("Use composition rectangle");
    GtkWindowMock *candidate_window_mock = new GtkWindowMock();
    GtkWindowMock *infolist_window_mock = new GtkWindowMock();
    GtkWrapperMock *gtk_mock = new GtkWrapperMock();

    commands::RendererCommand command;
    command.set_visible(true);
    commands::Candidates *candidates
        = command.mutable_output()->mutable_candidates();
    candidates->set_focused_index(0);

    commands::Candidates_Candidate *candidate = candidates->add_candidate();
    candidate->set_information_id(0);

    commands::InformationList *usage = candidates->mutable_usages();
    usage->add_information();

    const Rect client_cord_rect(10, 20, 30, 40);
    const Point window_position(15, 25);
    const Size window_size(35, 45);

    candidates->set_window_location(commands::Candidates::COMPOSITION);
    const Rect composition_rect(16, 26, 2, 13);
    commands::Rectangle *rectangle
        = candidates->mutable_composition_rectangle();
    rectangle->set_x(composition_rect.Left());
    rectangle->set_y(composition_rect.Top());
    rectangle->set_width(composition_rect.Width());
    rectangle->set_height(composition_rect.Height());
    const Point expected_window_position(
        composition_rect.Left() - client_cord_rect.Left(),
        composition_rect.Top() + composition_rect.Height());

    EXPECT_CALL(*candidate_window_mock, Update(_))
        .WillOnce(Return(window_size));
    EXPECT_CALL(*candidate_window_mock, GetCandidateColumnInClientCord())
        .WillOnce(Return(client_cord_rect));
    EXPECT_CALL(*candidate_window_mock, GetWindowPos())
        .WillOnce(Return(window_position));
    EXPECT_CALL(*candidate_window_mock,
                Move(PointEq(expected_window_position)));
    EXPECT_CALL(*candidate_window_mock, ShowWindow());

    GtkWidget *toplevel_widget = reinterpret_cast<GtkWidget*>(0x12345678);
    GdkScreen *toplevel_screen = reinterpret_cast<GdkScreen*>(0x87654321);
    const Rect screen_rect(0, 0, 4000, 4000);
    EXPECT_CALL(*gtk_mock, GtkWindowNew(GTK_WINDOW_TOPLEVEL))
        .WillOnce(Return(toplevel_widget));
    EXPECT_CALL(*gtk_mock, GtkWindowGetScreen(toplevel_widget))
        .WillOnce(Return(toplevel_screen));
    EXPECT_CALL(*gtk_mock, GdkScreenGetWidth(toplevel_screen))
        .WillOnce(Return(screen_rect.size.width));
    EXPECT_CALL(*gtk_mock, GdkScreenGetHeight(toplevel_screen))
        .WillOnce(Return(screen_rect.size.height));
    WindowManager manager(candidate_window_mock, infolist_window_mock,
                          gtk_mock);
    const Rect actual_rect = manager.UpdateCandidateWindow(command);

    EXPECT_EQ(actual_rect.origin.x, expected_window_position.x);
    EXPECT_EQ(actual_rect.origin.y, expected_window_position.y);
    EXPECT_EQ(actual_rect.size.width, window_size.width);
    EXPECT_EQ(actual_rect.size.height, window_size.height);
  }
  {
    SCOPED_TRACE("Edge fixing");
    GtkWindowMock *candidate_window_mock = new GtkWindowMock();
    GtkWindowMock *infolist_window_mock = new GtkWindowMock();
    GtkWrapperMock *gtk_mock = new GtkWrapperMock();

    commands::RendererCommand command;
    commands::Candidates *candidates
        = command.mutable_output()->mutable_candidates();
    candidates->add_candidate();

    GtkWidget *toplevel_widget = reinterpret_cast<GtkWidget*>(0x12345678);
    GdkScreen *toplevel_screen = reinterpret_cast<GdkScreen*>(0x87654321);
    const Rect client_cord_rect(0, 0, 30, 40);
    const Point window_position(1000, 1000);
    const Size window_size(300, 400);
    const Rect screen_rect(0, 0, 1200, 1200);

    EXPECT_CALL(*candidate_window_mock, Update(_))
        .WillOnce(Return(window_size));
    EXPECT_CALL(*candidate_window_mock, GetCandidateColumnInClientCord())
        .WillOnce(Return(client_cord_rect));
    EXPECT_CALL(*candidate_window_mock, GetWindowPos())
        .WillOnce(Return(window_position));
    EXPECT_CALL(*candidate_window_mock, Move(_));
    EXPECT_CALL(*candidate_window_mock, ShowWindow());

    EXPECT_CALL(*gtk_mock, GtkWindowNew(GTK_WINDOW_TOPLEVEL))
        .WillOnce(Return(toplevel_widget));
    EXPECT_CALL(*gtk_mock, GtkWindowGetScreen(toplevel_widget))
        .WillOnce(Return(toplevel_screen));
    EXPECT_CALL(*gtk_mock, GdkScreenGetWidth(toplevel_screen))
        .WillOnce(Return(screen_rect.size.width));
    EXPECT_CALL(*gtk_mock, GdkScreenGetHeight(toplevel_screen))
        .WillOnce(Return(screen_rect.size.height));
    WindowManager manager(candidate_window_mock, infolist_window_mock,
                          gtk_mock);
    const Rect actual_rect = manager.UpdateCandidateWindow(command);

    EXPECT_LE(actual_rect.Right(), screen_rect.Right());
    EXPECT_LE(actual_rect.Bottom(), screen_rect.Bottom());
    EXPECT_GE(actual_rect.Left(), screen_rect.Left());
    EXPECT_GE(actual_rect.Top(), screen_rect.Top());
  }
}

TEST(WindowManagerTest, UpdateInfolistWindowTest) {
  {
    SCOPED_TRACE("If there are no information, should hide and do nothing");
    GtkWindowMock *candidate_window_mock = new StrictMock<GtkWindowMock>();
    GtkWindowMock *infolist_window_mock = new StrictMock<GtkWindowMock>();
    GtkWrapperMock *gtk_mock = new StrictMock<GtkWrapperMock>();

    commands::RendererCommand command;
    command.set_visible(false);

    EXPECT_CALL(*infolist_window_mock, HideWindow());
    const Rect candidate_window_rect(10, 20, 30, 40);

    WindowManager manager(candidate_window_mock, infolist_window_mock,
                          gtk_mock);
    manager.UpdateInfolistWindow(command, candidate_window_rect);
  }
  {
    GtkWindowMock *candidate_window_mock = new GtkWindowMock();
    GtkWindowMock *infolist_window_mock = new GtkWindowMock();
    GtkWrapperMock *gtk_mock = new GtkWrapperMock();

    commands::RendererCommand command;
    command.set_visible(true);
    commands::Candidates *candidates
        = command.mutable_output()->mutable_candidates();
    commands::Candidates_Candidate *candidate = candidates->add_candidate();

    commands::InformationList *usage = candidates->mutable_usages();
    usage->add_information();

    candidates->set_focused_index(0);
    candidate->set_information_id(0);


    GtkWidget *toplevel_widget = reinterpret_cast<GtkWidget*>(0x12345678);
    GdkScreen *toplevel_screen = reinterpret_cast<GdkScreen*>(0x87654321);
    const Rect screen_rect(15, 25, 35, 45);
    EXPECT_CALL(*gtk_mock, GtkWindowNew(GTK_WINDOW_TOPLEVEL))
        .WillOnce(Return(toplevel_widget));
    EXPECT_CALL(*gtk_mock, GtkWindowGetScreen(toplevel_widget))
        .WillOnce(Return(toplevel_screen));
    EXPECT_CALL(*gtk_mock, GdkScreenGetWidth(toplevel_screen))
        .WillOnce(Return(screen_rect.size.width));
    EXPECT_CALL(*gtk_mock, GdkScreenGetHeight(toplevel_screen))
        .WillOnce(Return(screen_rect.size.height));

    // TODO(nona): Make precise expectation.
    const Size infolist_window_size(10, 20);
    EXPECT_CALL(*infolist_window_mock, Move(_));
    EXPECT_CALL(*infolist_window_mock, ShowWindow());
    EXPECT_CALL(*infolist_window_mock, HideWindow()).Times(0);
    EXPECT_CALL(*infolist_window_mock, Update(_))
        .WillOnce(Return(infolist_window_size));

    const Rect candidate_window_rect(10, 20, 30, 40);
    WindowManager manager(candidate_window_mock, infolist_window_mock,
                          gtk_mock);

    EXPECT_TRUE(manager.ShouldShowInfolistWindow(command));
    manager.UpdateInfolistWindow(command, candidate_window_rect);
  }
}

TEST(WindowManagerTest, GetDesktopRectTest) {
  GtkWindowMock *candidate_window_mock = new GtkWindowMock();
  GtkWindowMock *infolist_window_mock = new GtkWindowMock();
  GtkWrapperMock *gtk_mock = new GtkWrapperMock();
  GtkWidget *toplevel_widget = reinterpret_cast<GtkWidget*>(0x12345678);
  GdkScreen *toplevel_screen = reinterpret_cast<GdkScreen*>(0x87654321);
  const Size screen_size(35, 45);
  EXPECT_CALL(*gtk_mock, GtkWindowNew(GTK_WINDOW_TOPLEVEL))
      .WillOnce(Return(toplevel_widget));
  EXPECT_CALL(*gtk_mock, GtkWindowGetScreen(toplevel_widget))
      .WillOnce(Return(toplevel_screen));
  EXPECT_CALL(*gtk_mock, GdkScreenGetWidth(toplevel_screen))
      .WillOnce(Return(screen_size.width));
  EXPECT_CALL(*gtk_mock, GdkScreenGetHeight(toplevel_screen))
      .WillOnce(Return(screen_size.height));
  WindowManager manager(candidate_window_mock, infolist_window_mock, gtk_mock);

  const Rect actual_screen_rect = manager.GetDesktopRect();
  EXPECT_EQ(0, actual_screen_rect.origin.x);
  EXPECT_EQ(0, actual_screen_rect.origin.y);
  EXPECT_EQ(screen_size.width, actual_screen_rect.size.width);
  EXPECT_EQ(screen_size.height, actual_screen_rect.size.height);
}

class FontUpdateTestableWindowManager : public WindowManager {
 public:
  FontUpdateTestableWindowManager(GtkWindowInterface *main_window,
                                  GtkWindowInterface *infolist_window,
                                  GtkWrapperInterface *gtk)
      : WindowManager(main_window, infolist_window, gtk) {}
  virtual ~FontUpdateTestableWindowManager() {}

  MOCK_METHOD1(ShouldShowCandidateWindow,
               bool(const commands::RendererCommand &command));
  MOCK_METHOD1(ShouldShowInfolistWindow,
               bool(const commands::RendererCommand &command));
  MOCK_METHOD1(UpdateCandidateWindow,
               Rect(const commands::RendererCommand &command));
  MOCK_METHOD2(UpdateInfolistWindow,
               void(const commands::RendererCommand &command,
                    const Rect &candidate_window_rect));
};

TEST(WindowManagerTest, FontReloadTest) {
  {
    GtkWindowMock *candidate_window_mock = new GtkWindowMock();
    GtkWindowMock *infolist_window_mock = new GtkWindowMock();
    GtkWrapperMock *gtk_mock = new GtkWrapperMock();

    const Rect kDummyRect(0, 0, 0, 0);
    const char kDummyFontDescription[] = "Foo,Bar,Baz";

    FontUpdateTestableWindowManager window_manager(
        candidate_window_mock, infolist_window_mock, gtk_mock);
    EXPECT_CALL(window_manager, ShouldShowCandidateWindow(_))
        .WillOnce(Return(true));
    EXPECT_CALL(*candidate_window_mock,
                ReloadFontConfig(StrEq(kDummyFontDescription)));
    EXPECT_CALL(*infolist_window_mock,
                ReloadFontConfig(StrEq(kDummyFontDescription)));
    EXPECT_CALL(window_manager, UpdateCandidateWindow(_))
        .WillOnce(Return(kDummyRect));

    EXPECT_CALL(window_manager, UpdateInfolistWindow(_, _));

    commands::RendererCommand command;

    commands::RendererCommand::ApplicationInfo *app_info =
        command.mutable_application_info();
    app_info->set_pango_font_description(kDummyFontDescription);

    window_manager.UpdateLayout(command);
  }
  {
    SCOPED_TRACE("Does not call reload function when the custom font setting "
                 "is not available");
    GtkWindowMock *candidate_window_mock = new GtkWindowMock();
    GtkWindowMock *infolist_window_mock = new GtkWindowMock();
    GtkWrapperMock *gtk_mock = new GtkWrapperMock();

    const Rect kDummyRect(0, 0, 0, 0);

    FontUpdateTestableWindowManager window_manager(
        candidate_window_mock, infolist_window_mock, gtk_mock);
    EXPECT_CALL(window_manager, ShouldShowCandidateWindow(_))
        .WillOnce(Return(true));
    EXPECT_CALL(*candidate_window_mock, ReloadFontConfig(_)).Times(0);
    EXPECT_CALL(*infolist_window_mock, ReloadFontConfig(_)).Times(0);
    EXPECT_CALL(window_manager, UpdateCandidateWindow(_))
        .WillOnce(Return(kDummyRect));

    EXPECT_CALL(window_manager, UpdateInfolistWindow(_, _));

    commands::RendererCommand command;
    command.mutable_application_info();

    window_manager.UpdateLayout(command);
  }
  {
    SCOPED_TRACE("Does not call reload function if previously loaded font"
                 "description is same as requested one.");
    GtkWindowMock *candidate_window_mock = new GtkWindowMock();
    GtkWindowMock *infolist_window_mock = new GtkWindowMock();
    GtkWrapperMock *gtk_mock = new GtkWrapperMock();

    const Rect kDummyRect(0, 0, 0, 0);
    const char kDummyFontDescription[] = "Foo,Bar,Baz";

    FontUpdateTestableWindowManager window_manager(
        candidate_window_mock, infolist_window_mock, gtk_mock);
    EXPECT_CALL(window_manager, ShouldShowCandidateWindow(_))
        .Times(2)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*candidate_window_mock,
                ReloadFontConfig(StrEq(kDummyFontDescription)));
    EXPECT_CALL(*infolist_window_mock,
                ReloadFontConfig(StrEq(kDummyFontDescription)));
    EXPECT_CALL(window_manager, UpdateCandidateWindow(_))
        .Times(2)
        .WillRepeatedly(Return(kDummyRect));

    EXPECT_CALL(window_manager, UpdateInfolistWindow(_, _)).Times(2);

    commands::RendererCommand command;

    commands::RendererCommand::ApplicationInfo *app_info =
        command.mutable_application_info();
    app_info->set_pango_font_description(kDummyFontDescription);

    window_manager.UpdateLayout(command);

    // Call again with same font description.
    window_manager.UpdateLayout(command);
  }
  {
    SCOPED_TRACE("Call reload function if previously loaded font description"
                 " is different from requested one.");
    GtkWindowMock *candidate_window_mock = new GtkWindowMock();
    GtkWindowMock *infolist_window_mock = new GtkWindowMock();
    GtkWrapperMock *gtk_mock = new GtkWrapperMock();

    const Rect kDummyRect(0, 0, 0, 0);
    const char kDummyFontDescription[] = "Foo,Bar,Baz";
    const char kDummyFontDescription2[] = "Foo,Bar";

    FontUpdateTestableWindowManager window_manager(
        candidate_window_mock, infolist_window_mock, gtk_mock);
    EXPECT_CALL(window_manager, ShouldShowCandidateWindow(_))
        .Times(2)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*candidate_window_mock,
                ReloadFontConfig(StrEq(kDummyFontDescription)));
    EXPECT_CALL(*infolist_window_mock,
                ReloadFontConfig(StrEq(kDummyFontDescription)));
    EXPECT_CALL(*candidate_window_mock,
                ReloadFontConfig(StrEq(kDummyFontDescription2)));
    EXPECT_CALL(*infolist_window_mock,
                ReloadFontConfig(StrEq(kDummyFontDescription2)));
    EXPECT_CALL(window_manager, UpdateCandidateWindow(_))
        .Times(2)
        .WillRepeatedly(Return(kDummyRect));

    EXPECT_CALL(window_manager, UpdateInfolistWindow(_, _)).Times(2);

    commands::RendererCommand command;

    commands::RendererCommand::ApplicationInfo *app_info =
        command.mutable_application_info();
    app_info->set_pango_font_description(kDummyFontDescription);

    window_manager.UpdateLayout(command);

    app_info->set_pango_font_description(kDummyFontDescription2);
    // Call again with different font description.
    window_manager.UpdateLayout(command);
  }
}
}  // namespace gtk
}  // namespace renderer
}  // namespace mozc
