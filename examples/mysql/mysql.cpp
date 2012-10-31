//
// Title: List of sexes from a public human genome database
// 
// Author: Karl Yerkes, 2012-10-30
//
// Description: This example queries a public MySQL database for a list of
// 'sexes', then visualizes the results. It should serve as an example of how
// to use an external c++ library (mysql++) with an AlloSystem project. It also
// shows how to use al::Font.
// 

#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_Font.hpp"
using namespace al;

#include <mysql++/mysql++.h>
using namespace std;

const char* db = "", *server = "genome-mysql.cse.ucsc.edu", *user = "genome", *pass = "";

struct MySQL : App {

  double time;
  mysqlpp::StoreQueryResult result;

  MySQL() {
    time = 0;

    mysqlpp::Connection connection(false);
    if (! connection.connect(db, server, user, pass)) {
      cerr << "connection failed: " << connection.error() << endl;
      exit(1);
    }

    mysqlpp::Query query = connection.query("select * from hg19.sex");
    if (! (result = query.store())) {
      cerr << "query failed: " << query.error() << endl;
      exit(1);
    }

    initWindow();
  }

  virtual void onAnimate(double dt) {
    time += dt;
  }

  virtual void onDraw(Graphics& g, const Viewpoint& v) {
    static Font font("../../examples/mysql/Inconsolata.otf", 72);
    g.clearColor(0.1, 0.1, 0.1, 1);
    g.clear(Graphics::COLOR_BUFFER_BIT);
    g.depthMask(false);
    g.depthTesting(false);
    g.blending(true);
    g.blendModeAdd();
    for (size_t row = 0; row < result.num_rows(); ++row) {
      g.pushMatrix();
        g.rotate((float)row / result.num_rows() * 360 + time, 1, 0, 0);
        g.translate(-1.0, 0, -3.2);
        g.scale(0.001);
        font.render(g, result[row][1].c_str());
      g.popMatrix();
    }
  }
};

int main(){
  MySQL().start();
}
