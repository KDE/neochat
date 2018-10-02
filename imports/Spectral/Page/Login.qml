import QtQuick 2.9

LoginForm {
    loginButton.onClicked: {
        if (!(serverField.text.startsWith("http") && serverField.text.includes("://"))) {
            loginButtonTooltip.text = "Server address should start with http(s)://"
            loginButtonTooltip.open()
            return
        }
        if (!(usernameField.text.startsWith("@") && usernameField.text.includes(":"))) {
            loginButtonTooltip.text = "Username should be in format of @example:example.com"
            loginButtonTooltip.open()
            return
        }

        controller.loginWithCredentials(serverField.text, usernameField.text, passwordField.text)

        controller.connectionAdded.connect(function() { stackView.pop() })
    }
}
