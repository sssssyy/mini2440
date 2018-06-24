#ifndef MYHELLOFORM_H
#define MYHELLOFORM_H
#include "hello_base.h"
#include <qpixmap.h>


class MyHelloForm : public HelloBaseForm
{ 
	Q_OBJECT
	
	public:
	    MyHelloForm( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
	    virtual ~MyHelloForm();

	    QTimer *timer;	    

	private slots:
	    void showMe();
	    void camara_quit();
	    void camara_snap();
	    
	signals:
	    void quit_signal();
	    void close_signal();
	    void signal2show();
	    void signal2hide();
};


#endif // MYHELLOFORM_H
