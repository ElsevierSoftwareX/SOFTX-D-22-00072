from flask import Flask,send_file,render_template


app = Flask(__name__)

@app.route("/HiVecMap")
def returnHtml():
    return render_template("HiVecMap.html")


if __name__ == '__main__':
    app.run(host='localhost', port='8888', debug=True)