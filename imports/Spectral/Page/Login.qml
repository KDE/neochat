import QtQuick 2.9

LoginForm {
    loginButton.onClicked: doLogin()

    Shortcut {
        sequence: "Return"
        onActivated: doLogin()
    }

    function doLogin() {
        if (!(serverField.text.startsWith("http") && serverField.text.includes("://"))) {
            loginButtonTooltip.text = "Server address should start with http(s)://"
            loginButtonTooltip.open()
            return
        }

        loginButton.text = "Logging in..."
        loginButton.enabled = false
        controller.loginWithCredentials(serverField.text, usernameField.text, passwordField.text)

        controller.connectionAdded.connect(function(conn) {
            stackView.pop()
            accountListView.currentConnection = conn
        })
    }
}
