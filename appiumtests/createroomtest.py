#!/usr/bin/env python3

# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>

import os
import subprocess
import sys
import unittest
import time

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy


class CreateRoomTest(unittest.TestCase):

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

    def test_create_room(self):
        self.driver.find_element(by=AppiumBy.NAME, value="@user:localhost:1234").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Show Menu").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Create a Room").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Name:").send_keys("Super awesome room name")#
        time.sleep(0.1) # without this, the second half of the text is sent to the topic field?!
        self.driver.find_element(by=AppiumBy.NAME, value="Topic:").send_keys("There are not enough raccoons here")
        time.sleep(0.1)
        self.driver.find_element(by=AppiumBy.NAME, value="Create Room").click()
        time.sleep(0.1)
        self.driver.find_element(by=AppiumBy.NAME, value="Super awesome room name").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Show Room Information").click()
        self.driver.find_element(by=AppiumBy.NAME, value="There are not enough raccoons here")


if __name__ == '__main__':
    unittest.main()
