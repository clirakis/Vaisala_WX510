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
 ===============================================================
"""
from flask import (
    Flask, render_template, send_file, make_response, request, redirect,
    url_for
    )
from flask_bootstrap import Bootstrap
from flask_moment import Moment
from datetime import datetime
import os
import numpy as np
from Plot.Graph import Graph

# My local imports
from PySM.Commands import Commands
from PySM.smWXT510 import smWXT510


app = Flask(__name__)

"""
Everything is global, probably ought to tighten this up.
"""

# This is straight from the documentation and should work. 
bootstrap = Bootstrap(app)
moment    = Moment(app)
# create the classes
R0    = smWXT510()
COMM  = Commands()

# Global Variable
CurrentFileName = "NONE"

"""
Initialize the ploting package.
"""
MyGraph = Graph()

@app.errorhandler(404)
def page_not_found(e):
    return render_template('404.html'), 404

@app.errorhandler(500)
def internal_server_error(e):
    return render_template('500.html'), 500

@app.route('/')
def index():
    return render_template('index.html',current_time=datetime.utcnow()) 

@app.route('/user')
def user():
    return render_template('user.html',current_time=datetime.utcnow())
@app.route('/Weather', methods=['GET','POST'])
def imu():
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
##    MyIMU.Read()

##    sTemperature = '{:03.1f}'.format(MyIMU.fTemperature)
##    sAx = '{:06.6f}'.format(MyIMU.fAcc[0])
##    sAy = '{:06.6f}'.format(MyIMU.fAcc[1])
##    sAz = '{:06.6f}'.format(MyIMU.fAcc[2])
##    sGx = '{:06.6f}'.format(MyIMU.fGyro[0])
##    sGy = '{:06.6f}'.format(MyIMU.fGyro[1])
##    sGz = '{:06.6f}'.format(MyIMU.fGyro[2])
##    sMx = '{:06.6f}'.format(MyIMU.fMagnetic[0])
##    sMy = '{:06.6f}'.format(MyIMU.fMagnetic[1])
##    sMz = '{:06.6f}'.format(MyIMU.fMagnetic[2])

    
##    return render_template('IMU.html',
##                           current_time=datetime.utcnow(),
##                           IMUTime=datetime.utcnow(),
##                           Fraction=0.1,
##                           Temperature=sTemperature,
##                           AX=sAx,AY=sAy,AZ=sAz,
##                           GX=sGx,GY=sGy,GZ=sGz,
##                           MX=sMx,MY=sMy,MZ=sMz,
##                           )
    return render_template('IMU.html')


@app.route('/testme')
def testme():
    return render_template('testme.html')

##@app.route('/plotGPS')
##def plotGPS():
##    global MyGraph
##    graphdata = MyGraph.InlinePlot()
##    response = make_response(graphdata.getvalue())
##    response.mimetype = 'image/png'
##    return response

if __name__ == "__main__":
    """
    This is our main entry point. I wonder if I could define all the
    classes here.
    """
    # put my initialization in here. This one  
    #
    # Plotting tools.
    #
    moment.init_app(app)
    app.run(host='0.0.0.0', port=5000, debug=True)
