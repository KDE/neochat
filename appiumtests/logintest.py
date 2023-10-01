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


class LoginTest(unittest.TestCase):

    mockServerProcess: subprocess.Popen

    @classmethod
    def setUpClass(cls):
        options = AppiumOptions()
        options.set_capability("app", "neochat --ignore-ssl-errors")
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)
        cls.mockServerProcess = subprocess.Popen([sys.executable, os.path.join(os.path.dirname(__file__), "login-server.py")])

    def setUp(self):
        pass

    def tearDown(self):
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file("failed_test_shot_{}.png".format(self.id()))

    @classmethod
    def tearDownClass(self):
        self.mockServerProcess.terminate()
        self.driver.quit()

    def test_login(self):
        self.driver.find_element(by=AppiumBy.NAME, value="Login").click()

        self.driver.find_element(by=AppiumBy.NAME, value="Matrix ID").send_keys("@user:localhost:1234")
        self.driver.find_element(by=AppiumBy.NAME, value="Continue").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Password").send_keys("1234")
        self.driver.find_element(by=AppiumBy.NAME, value="Login").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Join some rooms to get started").click()


if __name__ == '__main__':
    unittest.main()
