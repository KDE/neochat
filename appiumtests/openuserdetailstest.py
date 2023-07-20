#!/usr/bin/env python3

# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>
# SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>

import os
import subprocess
import sys
import unittest

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy


class OpenUserDetailsTest(unittest.TestCase):

    mockServerProcess: subprocess.Popen

    @classmethod
    def setUpClass(cls):
        cls.mockServerProcess = subprocess.Popen([sys.executable, os.path.join(os.path.dirname(__file__), "login-server.py")])
        options = AppiumOptions()
        options.set_capability("app", "neochat --ignore-ssl-errors --test")
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    def setUp(self):
        pass

    def tearDown(self):
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file("failed_test_shot_{}.png".format(self.id()))

    @classmethod
    def tearDownClass(self):
        self.mockServerProcess.terminate()
        self.driver.quit()

    def test_open_sheet(self):
        self.driver.find_element(by=AppiumBy.NAME, value="@user:localhost:1234").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Empty room (!room_id_1234:localhost:1234)").click()
        self.driver.find_element(by=AppiumBy.NAME, value="A Display Name").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Account Details")


if __name__ == '__main__':
    unittest.main()
