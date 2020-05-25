from flask import Flask
from flask_cors import CORS

# Import views
from .views import eeg_data_server

def create_app():
    app = Flask(__name__)

    # Enable cross-origin resource sharing
    #
    # NB: This is here so that we are able to make XMLHttpRequests from an
    #   independent frontend server during the development phase. Later on,
    #   we can re-think how we want the frontend and backend to interact.
    CORS(app)

    # Register blueprints
    app.register_blueprint(eeg_data_server)

    return app
