"""
 ===============================================================
Modified    By    Reason
--------    --    ------
03-Jun-26   CBL   Moved over from PiDA and using here. 

Bootstrap has a lot of stuff in it, basic templates
moment is for dealing with time 

References:
https://pypi.org/project/Flask-Bootstrap/
https://flask-moment.readthedocs.io/en/latest/quickstart.html#rendering-timestamps-with-flask-moment
https://getbootstrap.com/docs/3.4/css/
https://flask.palletsprojects.com/en/stable/tutorial/factory/
 ===============================================================
"""
from flask import (
    Flask, render_template, send_file, make_response, request, redirect,
    url_for
    )
from flask_bootstrap import Bootstrap
from flask_moment import Moment
import logging
from datetime import datetime
import os
import signal
import sys
import numpy as np
from Plot.Graph import Graph

# My local imports
from PySM.Commands import Commands
from PySM.smWXT510 import smWXT510
from WX.wxt510 import wxt510

"""
This is a lot more clean than the way I was doing initialization!!
08-Jun-26
"""
logfile = open('sensor.log','w') 

def signal_handler(sig, frame):
    print('Termination signal received! Shutting down gracefully...')
    # Close resources
    #del pwxt
    logfile.close()

    sys.exit(0)

def create_app(test_config=None):
    # create and configure the app
    app = Flask(__name__, instance_relative_config=True)

    # ensure the instance folder exists
    #os.makedirs(app.instance_path, exist_ok=True)

    # Register handlers for SIGINT (Ctrl+C) and SIGTERM
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    # dial back the logging
    log = logging.getLogger('werkzeug')
    log.setLevel(logging.ERROR)
    
    bootstrap = Bootstrap(app)
    moment    = Moment(app)

    pR0   = smWXT510()
    COMM  = Commands()
    pwxt  = wxt510()

    logfile.write("# ---------------------------------------------\n")
    logfile.write("# SYS-I-FILE: Weather.py \n");
    logfile.write('# SYS-I-FILE: Logfile started: '+str(datetime.utcnow())+'\n')
    if (pR0.error == 0):
        logfile.write("# Shared memory attached.\n")
    else:
        logfile.write("# Could not attach to shared memory.\n")
        
    pwxt.SetLogfile(logfile)

    # Global Variable
    CurrentFileName = "NONE"


    """
    Initialize the ploting package.
    """
    #MyGraph = Graph()


    @app.errorhandler(404)
    def page_not_found(e):
        return render_template('404.html'), 404

    @app.errorhandler(500)
    def internal_server_error(e):
        return render_template('500.html'), 500
    
    # a simple page that says hello
    @app.route('/hello')
    def hello():
        return 'Hello, World!'

    @app.route('/')
    def index():
        return render_template('index.html',current_time=datetime.utcnow()) 

    @app.route('/testme')
    def testme():
        return render_template('testme.html')
        return app


    # This gets called every time a page finishes. Not when the app ends
##    @app.teardown_appcontext
##    def teardown_db(exception):
##        # Perform cleanup operations here
##        print("Application context is ending...")
##        #logfile.write('# Program end: '+str(datetime.utcnow())+'\n')
##        #logfile.close()
        
    @app.route('/user')
    def user():
        return render_template('user.html',current_time=datetime.utcnow())

    @app.route('/wx', methods=['GET','POST'])
    def wx():
        # Turn on for debugging prints. 
        if (False):
            if request.method == 'POST':
                # POST is the form sent some data. 
                print('Weather POST')

                if request.form.get('refresh') == 'Refresh':
                    print("Weather do refresh")
                else:
                    print("method unknown")

            elif request.method == 'GET':
                # GET is a give me some data
                print('Weather GET')

        if (pR0.error == 0):
            pR0.Read()
            input = pR0.R0
        else:
            input = "0R0,Dm=000#,Sm=0.0#,Ta=18.6C,Ua=59.4P,Pa=1.0141B,Rc=0.00M,Th=18.6C,Vh=13.7N\r\n"

    #    input = "0R0,Dm=000#,Sm=0.0#,Ta=18.6C,Ua=59.4P,Pa=1.0141B,Rc=0.00M,Th=18.6C,Vh=13.7N\r\n"
        pwxt.Decode(input,pwxt.logfile)

        # format the strings to pass to the template. 
        sTemperature   = '{:04.1f}'.format(pwxt.Temperature)
        sPressure      = '{:06.4f}'.format(pwxt.Pressure)
        sHumidity      = '{:06.6f}'.format(pwxt.Humidity)
        sSpeed         = '{:04.1f}'.format(pwxt.WindSpeed.Average)
        sDirection     = '{:06.1f}'.format(pwxt.WindDirection.Average)
        sRAccumulation = '{:06.2f}'.format(pwxt.Rain_accumulation)
        sRDuration     = '{:06.2f}'.format(pwxt.Rain_duration)
        sHAccumulation = '{:06.2f}'.format(pwxt.Hail_accumulation)
        sHDuration     = '{:06.2f}'.format(pwxt.Hail_duration)

        # 
        fTemperature   = '{:04.1f}'.format(pwxt.Temperature*9/5 + 32)

        return render_template('WX.html',
                               current_time=datetime.utcnow(),
                               WXTime=datetime.utcnow(),
                               Temperature=sTemperature,
                               Humidity=sHumidity,
                               Pressure=sPressure,
                               Speed=sSpeed,
                               Direction=sDirection,
                               RAccumulation=sRAccumulation,
                               RDuration=sRDuration,
                               HAccumulation=sHAccumulation,
                               HDuration=sHDuration,
                               FTemperature=fTemperature,
                               )



    return app
