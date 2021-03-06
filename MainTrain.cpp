
/*
 * run2.cpp
 *
 *  Created on: 8 ����� 2019
 *      Author: Eli
 */

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "Server.h"




using namespace std;

void writeStr(string input,int serverFD){
  write(serverFD,input.c_str(),input.length());
  write(serverFD,"\n",1);
}

string readStr(int serverFD){
  string serverInput="";
  char c=0;
  read(serverFD,&c,sizeof(char));
  while(c!='\n'){
    serverInput+=c;
    read(serverFD,&c,sizeof(char));
  }
  return serverInput;
}

void readMenue(ofstream& out,int serverFD){
  bool done=false;
  while(!done){
    // read string line
    string serverInput = readStr(serverFD);
    if(serverInput=="6.exit")
      done=true;
    out<<serverInput<<endl;
  }
}

int initClient(int port)throw (const char*){
  int serverFD, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  serverFD = socket(AF_INET, SOCK_STREAM, 0);
  if (serverFD < 0)
    throw "socket problem";

  server = gethostbyname("localhost");
  if(server==NULL)
    throw "no such host";

  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);

  serv_addr.sin_port = htons(port);
  //cout << to_string(serv_addr.sin_addr.s_addr);
    if (connect(serverFD,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
      throw "connection problem";

  return serverFD;
}

void clientSide1(int port,string outputFile)throw (const char*){
  int serverFD = initClient(port);
  ofstream out(outputFile);
  readMenue(out,serverFD);
  out.close();
  string input="6";
  writeStr(input,serverFD);
  close(serverFD);
  cout<<"end of client 1"<<endl;
}


void clientSide2(int port,string outputFile)throw (const char*){

  int serverFD = initClient(port);

  ofstream out(outputFile);
  ifstream in("input.txt");
  string input="";
  while(input!="6"){
    readMenue(out,serverFD);
    getline(in,input);
    writeStr(input,serverFD);
    if(input=="1"){
      out<<readStr(serverFD)<<endl; // please upload...
      while(input!="done"){
        getline(in,input);
        writeStr(input,serverFD);
      }
      out<<readStr(serverFD)<<endl; // Upload complete
      out<<readStr(serverFD)<<endl; // please upload...
      input="";
      while(input!="done"){
        getline(in,input);
        writeStr(input,serverFD);
      }
      out<<readStr(serverFD)<<endl; // Upload complete
    }

    if(input=="3"){
      out<<readStr(serverFD)<<endl; // Anomaly... complete
    }
    if(input=="5"){
      out<<readStr(serverFD)<<endl; // please upload...
      while(input!="done"){
        getline(in,input);
        writeStr(input,serverFD);
      }
      out<<readStr(serverFD)<<endl; // Upload complete
      out<<readStr(serverFD)<<endl; // TPR
      out<<readStr(serverFD)<<endl; // FPR
    }
  }
  in.close();
  out.close();

  close(serverFD);
  cout<<"end of client 2"<<endl;
}

size_t check(string outputFile,string expectedOutputFile){
  ifstream st(outputFile);
  ifstream ex(expectedOutputFile);
  size_t i=0;
  string lst,lex;
  while(!ex.eof()){
    getline(st,lst);
    getline(ex,lex);
    if(lst.compare(lex)!=0)
      i++;
  }
  st.close();
  ex.close();
  return i;
}


int main(){
  srand (time(NULL));
  int port=5000+ rand() % 1000;
  string outputFile1="output_menu";
  string outputFile2="output";
  int x=rand() % 1000;
  outputFile1+=to_string(x);
  outputFile1+=".txt";
  outputFile2+=to_string(x);
  outputFile2+=".txt";

  try{

    AnomalyDetectionHandler adh;
    Server server(port);
    server.start(adh); // runs on its own thread

    clientSide1(port,outputFile1);
    clientSide2(port,outputFile2);
    server.stop(); // joins the server's thread
  }catch(const char* s){
    cout<<s<<endl;
  }
  size_t mistakes = check(outputFile1,"expected_output_menu.txt");
  mistakes += check(outputFile2,"expected_output.txt");

  if(mistakes>0)
    cout<<"you have "<<mistakes<<" mistakes in your output (-"<<(mistakes*2)<<")"<<endl;

  cout<<"done"<<endl;
  return 0;
}


/*
 * MainTrain.cpp
 *
 *  Created on: 11 ����� 2020
 *      Author: Eli
 */
/*
#include <iostream>
#include <vector>
#include "HybridAnomalyDetector.h"
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include "commands.h"
#include "CLI.h"

using namespace std;


class STDtest:public DefaultIO{
    ifstream in;
    ofstream out;
public:
    STDtest(string inputFile,string outputFile){
        in.open(inputFile);
        out.open(outputFile);
    }
    virtual string read(){
        string s;
        in>>s;
        return s;
    }
    virtual void write(string text){
        out<<text;
    }

    virtual void write(float f){
        out<<f;
    }

    virtual void read(float* f){
        in>>*f;
    }

    void close(){
        if(in.is_open())
            in.close();
        if(out.is_open())
            out.close();
    }
    ~STDtest(){
        close();
    }
};

void check(string outputFile,string expectedOutputFile){
    size_t chk[]={31,62,63,74,75,86,87,98,99,110,111};
    ifstream st(outputFile);
    ifstream ex(expectedOutputFile);
    size_t i=1,j=0;
    string lst,lex;
    while(!st.eof() && !ex.eof()){
        getline(st,lst);
        getline(ex,lex);
        if(i<13 && lst.compare(lex)!=0){ // 12
            cout<<"line "<<i<<" expected: "<<lex<<" you got "<<lst<<endl;
            cout<<"wrong output (-1)"<<endl;
        }else
        if(j<11 && i==chk[j]){
            if(lst.compare(lex)!=0){ // 88
                cout<<"line "<<i<<" expected: "<<lex<<" you got "<<lst<<endl;
                cout<<"wrong output (-8)"<<endl;
            }
            j++;
        }
        i++;
    }
    if(j<11)
        cout<<"wrong output size (-"<<(11-j)*8<<")"<<endl;
    st.close();
    ex.close();
}

//small test
int main(){
    STDtest std("input.txt","output.txt");
    CLI cli(&std);
    cli.start();
    std.close();
    check("output.txt","expectedOutput.txt");
    cout<<"done"<<endl;
    return 0;
}
*/

/*
void generateTrainCSV(float a1,float b1, float a2, float b2, float a3, float b3){
    ofstream out("trainFile.csv");
    out<<"A,B,C,D,E,F"<<endl;
    Line ab(a1,b1);
    Line cd(a2,b2);
    Line ef(a3,b3);
    for(int i=1;i<=200;i++){
        float a=rand()%40;
        float b=ab.f(a)-0.03+(rand()%60)/100.0f;
        float c=rand()%40;
        float d=cd.f(c)-0.03+(rand()%60)/100.0f;
        float r=-100+rand()%200;
        float an=45-40+rand()%70;
        float e=r*cosf(3.14159f*(an)/180);
        float f=r*sinf(3.14159f*(an)/180);
        out<<a<<","<<b<<","<<c<<","<<d<<","<<e<<","<<f<<endl;
    }
    out.close();
}

vector<AnomalyReport> anomalies;
void generateTestCSV(float a1,float b1, float a2, float b2, float a3, float b3){
    ofstream out("testFile.csv");
    out<<"A,B,C,D,E,F"<<endl;
    Line ab(a1,b1);
    Line cd(a2,b2);
    Line ef(a3,b3);
    for(int i=1;i<=200;i++){
        float a=rand()%40;
        float b=ab.f(a)-0.03+(rand()%60)/100.0f;
        float c=rand()%40;
        float d=cd.f(c)-0.03+(rand()%60)/100.0f;
        float r=-100+rand()%200;

        for(auto it=anomalies.begin();it!=anomalies.end();it++){
            if((*it).timeStep==i){
                if((*it).description=="A-B")
                    b++;
                if((*it).description=="C-D")
                    d++;
                if((*it).description=="E-F")
                    r=111;
            }
        }
        float an=45-40+rand()%70;
        float e=r*cosf(3.14159f*(an)/180);
        float f=r*sinf(3.14159f*(an)/180);

        out<<a<<","<<b<<","<<c<<","<<d<<","<<e<<","<<f<<endl;

    }
    out.close();
}

void checkCorrelation(correlatedFeatures c,string f1, string f2, float a, float b){
    if(c.feature1==f1){
        if(c.feature2!=f2)
            cout<<"wrong correlated feature of "<<f1<<" (-10)"<<endl;
        else{
            if(c.corrlation>=0.9){
                if(c.corrlation<0.99)
                    cout<<f1<<"-"<<f2<<" wrong correlation detected (-2)"<<endl;
                if(c.lin_reg.a<a-0.5f || c.lin_reg.a>a+0.5f)
                    cout<<f1<<"-"<<f2<<" wrong value of line_reg.a (-4)"<<endl;
                if(c.lin_reg.b<b-0.5f || c.lin_reg.b>b+0.5f)
                    cout<<f1<<"-"<<f2<<" wrong value of line_reg.b (-4)"<<endl;
            }else{
                if(c.corrlation<=0.5)
                    cout<<f1<<"-"<<f2<<" wrong correlation detected (-2)"<<endl;
                if(c.threshold>111)
                    cout<<f1<<"-"<<f2<<" wrong value of the radius (-18)"<<endl;
            }
        }
    }

}

int main(){
    srand (time(NULL));
    float a1=1+rand()%10, b1=-50+rand()%100;
    float a2=1+rand()%20 , b2=-50+rand()%100;
    float a3=1+rand()%40 , b3=-50+rand()%100;


    // test the learned model: (40 points)
    // expected correlations:
    //	A-B: y=a1*x+b1
    //	C-D: y=a2*x+b2
    //	E-F: y=a3*x+b3

    generateTrainCSV(a1,b1,a2,b2,a3,b3);
    TimeSeries ts("trainFile.csv");
    HybridAnomalyDetector ad;
    ad.learnNormal(ts);
    vector<correlatedFeatures> cf=ad.getNormalModel();

    if(cf.size()!=3)
        cout<<"wrong size of correlated features (-40)"<<endl;
    else
        for_each(cf.begin(),cf.end(),[&a1,&b1,&a2,&b2,&a3,&b3](correlatedFeatures c){
            checkCorrelation(c,"A","B",a1,b1); // 10 points
            checkCorrelation(c,"C","D",a2,b2); // 10 points
            checkCorrelation(c,"E","F",a3,b3); // 20 points
        });

    // test the anomaly detector: (60 points)
    // one simply anomaly is injected to the data
    int anomaly1=5+rand()%90; // anomaly injected in a random time step
    int anomaly2=5+rand()%90; // anomaly injected in a random time step
    int anomaly3=5+rand()%90; // anomaly injected in a random time step

    anomalies.push_back(AnomalyReport("A-B",anomaly1));
    anomalies.push_back(AnomalyReport("C-D",anomaly2));
    anomalies.push_back(AnomalyReport("E-F",anomaly3));

    generateTestCSV(a1,b1,a2,b2,a3,b3);
    TimeSeries ts2("testFile.csv");
    vector<AnomalyReport> r = ad.detect(ts2);
    bool detected[]={false,false,false};

    for_each(r.begin(),r.end(),[&detected](AnomalyReport ar){
        if(ar.description==anomalies[0].description && ar.timeStep==anomalies[0].timeStep)
            detected[0]=true;
        if(ar.description==anomalies[1].description && ar.timeStep==anomalies[1].timeStep)
            detected[1]=true;
        if(ar.description==anomalies[2].description && ar.timeStep==anomalies[2].timeStep)
            detected[2]=true;
    });

    int falseAlarms=r.size();
    for(int i=0;i<3;i++)
        if(!detected[i])
            cout<<"an anomaly was not detected (-10)"<<endl;
        else
            falseAlarms--;

    if(falseAlarms>0)
        cout<<"you have "<<falseAlarms<<" false alarms (-"<<min(30,falseAlarms*3)<<")"<<endl;

    cout<<"done"<<endl;
    return 0;
}*/

/*#include <iostream>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "minCircle.h"
#include <chrono>

using namespace std;
using namespace std::chrono;


Point** generate(Point center,int R, size_t size){
    Point** p =new Point*[size];
    for(size_t i=0;i<size;i++){
        float r=1+rand()%R;
        float a=3.14159*(rand()%360)/180;
        float x=center.x+r*cos(a);
        float y=center.y+r*sin(a);
        p[i]=new Point(x,y);
    }
    return p;
}


int main(){
    srand (time(NULL));
    const size_t N=250;
    float R=10+rand()%1000;
    float cx=-500+rand()%1001;
    float cy=-500+rand()%1001;
    Point** ps=generate(Point(cx,cy),R,N);

    // your working copy
    Point** ps_copy=new Point*[N];
    for(size_t i=0;i<N;i++)
        ps_copy[i]=new Point(ps[i]->x,ps[i]->y);

    auto start = high_resolution_clock::now();
    Circle c=findMinCircle(ps_copy,N);
    auto stop = high_resolution_clock::now();

    if((int)c.radius>(int)R)
        cout<<"you need to find a minimal radius (-40)"<<endl;

    bool covered=true;
    for(size_t i=0;i<N && covered;i++){
        float x2=(c.center.x-ps[i]->x)*(c.center.x-ps[i]->x);
        float y2=(c.center.y-ps[i]->y)*(c.center.y-ps[i]->y);
        float d=sqrt(x2+y2);
        if(d>c.radius+1)
            covered=false;
    }
    if(!covered)
        cout<<"all points should be covered (-45)"<<endl;

    auto duration = duration_cast<microseconds>(stop - start);
    int stime=duration.count();
    cout<<"your time: "<<stime<<" microseconds"<<endl;
    if(stime>3000){
        cout<<"over time limit ";
        if(stime<=3500)
            cout<<"(-5)"<<endl;
        else if(stime<=4000)
            cout<<"(-8)"<<endl;
        else if(stime<=6000)
            cout<<"(-10)"<<endl;
        else cout<<"(-15)"<<endl;
    }

    for(size_t i=0;i<N;i++){
        delete ps[i];
        delete ps_copy[i];
    }
    delete[] ps;
    delete[] ps_copy;

    cout<<"done"<<endl;
    return 0;
}


/*
#include <iostream>
#include <vector>
#include "AnomalyDetector.h"
#include "SimpleAnomalyDetector.h"
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <math.h>

using namespace std;

// this is a simple test to put you on the right track
void generateTrainCSV(float a1,float b1, float a2, float b2){
    ofstream out("trainFile1.csv");
    out<<"A,B,C,D"<<endl;
    Line ac(a1,b1);
    Line bd(a2,b2);
    for(int i=1;i<=100;i++){
        float a=i;
        float b=rand()%40;
        out<<a<<","<<b<<","<<ac.f(a)-0.02+(rand()%40)/100.0f<<","<<bd.f(b)-0.02+(rand()%40)/100.0f<<endl;
    }
    out.close();
}

void generateTestCSV(float a1,float b1, float a2, float b2, int anomaly){
    ofstream out("testFile1.csv");
    out<<"A,B,C,D"<<endl;
    Line ac(a1,b1);
    Line bd(a2,b2);
    for(int i=1;i<=100;i++){
        float a=i;
        float b=rand()%40;
        if(i!=anomaly)
            out<<a<<","<<b<<","<<ac.f(a)-0.02+(rand()%40)/100.0f<<","<<bd.f(b)-0.02+(rand()%40)/100.0f<<endl;
        else
            out<<a<<","<<b<<","<<ac.f(a)+1<<","<<bd.f(b)-0.02+(rand()%40)/100.0f<<endl;
    }
    out.close();
}

void checkCorrelationTrain(correlatedFeatures c,string f1, string f2, float a, float b){
    if(c.feature1==f1){
        if(c.feature2!=f2)
            cout<<"wrong correlated feature of "<<f1<<" (-20)"<<endl;
        else{
          //  cout << "line_reg.a = " << c.lin_reg.a << endl;
           // cout << "line_reg.b = " << c.lin_reg.b << endl;

          if(c.corrlation<0.99)
                cout<<f1<<"-"<<f2<<" wrong correlation detected (-5)"<<endl;
            if(c.lin_reg.a<a-0.5f || c.lin_reg.a>a+0.5f)
                cout<<f1<<"-"<<f2<<" wrong value of line_reg.a (-5)" << " got: " << c.lin_reg.a <<endl;
            if(c.lin_reg.b<b-0.5f || c.lin_reg.b>b+0.5f)
                cout<<f1<<"-"<<f2<<" wrong value of line_reg.b (-5)" << " got: " << c.lin_reg.b<<endl ;
            if(c.threshold>0.3)
                cout<<f1<<"-"<<f2<<" wrong threshold detected (-5)"<<endl;
        }
    }

}

int main(){\
    srand (time(NULL));
    float a1=1+rand()%10, b1=-50+rand()%100;
    float a2=1+rand()%20 , b2=-50+rand()%100;


    // test the learned model: (40 points)
    // expected correlations:
    //	A-C: y=a1*x+b1
    //	B-D: y=a2*x+b2

    generateTrainCSV(a1,b1,a2,b2);
    TimeSeries ts("trainFile1.csv");

    SimpleAnomalyDetector ad;
    ad.learnNormal(ts);
    vector<correlatedFeatures> cf=ad.getNormalModel();
    if(cf.size()!=2)
        cout<<"wrong size of correlated features (-40)"<<endl;
    else
      for_each(cf.begin(),cf.end(),[&a1,&b1,&a2,&b2](correlatedFeatures c){
          checkCorrelationTrain(c,"A","C",a1,b1); // 20 points
          checkCorrelationTrain(c,"B","D",a2,b2); // 20 points
      });

  // test the anomaly detector: (60 points)
  // one simply anomaly is injected to the data
  int anomaly=5+rand()%90; // one anomaly injected in a random time step
  generateTestCSV(a1,b1,a2,b2,anomaly);

  TimeSeries ts2("testFile1.csv");
  vector<AnomalyReport> r = ad.detect(ts2);
  bool anomlyDetected=false;
  int falseAlarms=0;
  for_each(r.begin(),r.end(),[&anomaly,&anomlyDetected,&falseAlarms](AnomalyReport ar){
      if(ar.description=="A-C" && ar.timeStep == anomaly)
          anomlyDetected=true;
      else
          falseAlarms++;
  });

  if(!anomlyDetected)
      cout<<"the anomaly was not detected (-30)"<<endl;
  if(falseAlarms>0)
      cout<<"you have "<<falseAlarms<<" false alarms (-"<<min(30,falseAlarms*3)<<")"<<endl;

  cout<<"done"<<endl;
  return 0;

}

*/