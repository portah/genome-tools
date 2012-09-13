#include "Arguments.hpp"
#include <QCoreApplication>

// Global static pointer used to ensure a single instance of the class.
Arguments* volatile Arguments::m_pArgumentsInstance = 0; 
QSettings* Arguments::setup = 0;

QMutex Arguments::mutex;

/***************************************************************************************

****************************************************************************************/
QMap<QString,Arguments::_ArgDescr>& Arguments::getVarValStorage()
{
    static QMap<QString,_ArgDescr> m_VarValStorage;	
    return m_VarValStorage;
}

/***************************************************************************************

****************************************************************************************/
Arguments& Arguments::Instance()
{
    if(!m_pArgumentsInstance)
    {
        mutex.lock();
        if(!m_pArgumentsInstance)
        {
            //Arguments* volatile temp = 
            //      static_cast< Arguments* >(operator new (sizeof(Arguments)));
            m_pArgumentsInstance=new Arguments();
            setup=new QSettings();
            argsList();
        }
        mutex.unlock();
    }	
    return *m_pArgumentsInstance;
}

/***************************************************************************************

****************************************************************************************/
Arguments::Arguments()
{
}

Arguments::~Arguments()
{
}


/***************************************************************************************

****************************************************************************************/
void Arguments::Init(QStringList l)
{
	int correct=0;
	foreach(const QString &key,getVarValStorage().keys())
	{
		int index=-1;
		if(getVarValStorage()[key]._cname!="") 
		{
			if(getVarValStorage()[key]._type==QVariant::Bool)
			{
				index=l.indexOf(QRegExp("^\\-{1,2}"+getVarValStorage()[key]._cname+"$"),1);
			}
			else
			{
				index=l.indexOf(QRegExp("^\\-{1,2}"+getVarValStorage()[key]._cname+"=.+$"),1);
			}
			if(index>0)
			{
				QString command=l[index];

				if(command.left(2)=="--")
				{
					command.remove(0,2);
				}
				else if(command.left(1)=="-")
				{
					command.remove(0,1);
				}
				else
				{
					usage();
					throw "Incorrect parameter";
				}
				/*Selecting correct command*/
				if(getVarValStorage()[key]._type==QVariant::Bool && command==getVarValStorage()[key]._cname)
				{
					getVarValStorage()[key]._value=QVariant(true);
					/*probably save to the INI*/
					correct++;
					continue;
				}
				if( getVarValStorage()[key]._type==QVariant::String && 
					command.at(getVarValStorage()[key]._cname.size())==QChar('='))
				{
					command.remove(0,getVarValStorage()[key]._cname.size()+1);
					getVarValStorage()[key]._value=QVariant(command);
					correct++;
					continue;
				}
				if( getVarValStorage()[key]._type==QVariant::Int && 
					command.at(getVarValStorage()[key]._cname.size())==QChar('='))
				{
					command.remove(0,getVarValStorage()[key]._cname.size()+1);
					getVarValStorage()[key]._value=QVariant(command).toInt();
					correct++;
					continue;
				}
				usage();
				throw "Incorrect parameter";
			}
		}
            else if(!getVarValStorage()[key]._ininame.isEmpty() && setup->contains(getVarValStorage()[key]._ininame))
			{
				getVarValStorage()[key]._value=setup->value(getVarValStorage()[key]._ininame);
				continue;
			}
			else
			{
				if(getVarValStorage()[key]._required==true)
				{
					usage();
					throw "Required parameter not present";
				}

			}
	}
	if(correct!=l.size()-1)
	{
		usage();
		for(int i=0;i<l.size();i++)
		{
            cout<<"DEBUG: "<<l[i].toStdString()<<endl;
            }
		throw "Incorrect command line";
	}
}

/***************************************************************************************

****************************************************************************************/
void Arguments::argsList(void)
{
    Arguments::addArg("in","in","inFileName",QVariant::String,"input file name",QString("./input.bam"));
    Arguments::addArg("bedin","bedin","inBedFileName",QVariant::String,"input bedfile name",QString("./input.bed"));
    Arguments::addArg("batch","batch","batchFileName",QVariant::String,"SQL Batchfile name","");
    Arguments::addArg("out","out","outFileName",QVariant::String,"output file name",QString("./output"));
    Arguments::addArg("log","log","logFileName",QVariant::String,"log file name",QString("./logfile_def.log"));

    Arguments::addArg("sql_driver","","SQL/DRIVER",QVariant::String,"Database driver",QString("QMYSQL"));
    Arguments::addArg("sql_dbname","","SQL/DBNAME",QVariant::String,"Database name",QString("hg19"));
    Arguments::addArg("sql_host","sql_host","SQL/HOST",QVariant::String,"Database hostname",QString("10.200.42.25"));
    Arguments::addArg("sql_port","sql_port","SQL/PORT",QVariant::Int,"Database port",3306);
    Arguments::addArg("sql_user","sql_user","SQL/USER",QVariant::String,"Database user",QString("root"));
    Arguments::addArg("sql_pass","","SQL/PASS",QVariant::ByteArray,"Database pass",);
    Arguments::addArg("sql_query1","sql_query1","QUERIES/Q1",QVariant::String,"Q1","");
    Arguments::addArg("sql_query2","sql_query2","QUERIES/Q2",QVariant::String,"Q2","");
    Arguments::addArg("sql_query3","sql_query3","QUERIES/Q3",QVariant::String,"Q3","");
    Arguments::addArg("sql_query4","sql_query4","QUERIES/Q4",QVariant::String,"Q4","");
    Arguments::addArg("sql_query5","sql_query5","QUERIES/Q5",QVariant::String,"Q5","");

    Arguments::addArg("bed_window","bed_window","BED/WINDOW",QVariant::Int,"Window for counting ",20);
    Arguments::addArg("bed_siteshift","bed_siteshift","BED/SITESHIFT",QVariant::Int,"Bed graph reads shifting",0);
    Arguments::addArg("bed_format","bed_format","BED/FORMAT",QVariant::Int,"",4);
    Arguments::addArg("bed_separatestrand","bed_separatestrand","BED/SEPARATESTRAND",QVariant::Bool,"",false);
    Arguments::addArg("bed_HeaderString","bed_HeaderString","BED/HEADERSTRING",QVariant::String,"",QString("track type=bedGraph name=%1"));
    Arguments::addArg("no-bed-file","no-bed-file","",QVariant::Bool,"Do not create bed file",false);

    Arguments::addArg("sql_table","sql_table","",QVariant::String,"Sql table to create for bed graph","");
    Arguments::addArg("sql_grp","sql_grp","",QVariant::String,"Sql group for trackDb table","");

    Arguments::addArg("rna_seq","rna_seq","rnaseq",QVariant::String,"","");

    Arguments::addArg("avd_window","avd_window","AVD/WINDOW",QVariant::Int,"Average tag density window",2000);
    Arguments::addArg("avd_smooth","avd_smooth","AVD/SMOOTH",QVariant::Int,"Average smooth window (odd)",11);

    Arguments::addArg("plot_ext","plot_ext","",QVariant::String,"","");
    Arguments::addArg("gnuplot","gnuplot","",QVariant::String,"Path to gnuplot",QString("gnuplot.exe"));
    
    Arguments::addArg("window","window","WINDOW",QVariant::Int,"Window",2000);

    Arguments::addArg("threads","threads","",QVariant::Int,"Max number of threads",4);

    Arguments::addArg("sam_siteshift","sam_siteshift","SAM/SITESHIFT",QVariant::Int,"default siteshift",0);
    Arguments::addArg("sam_twicechr","sam_twicechr","SAM/TWICECHR",QVariant::String,"Which chromosome to double",QString(""));// chrX chrY
    Arguments::addArg("sam_ignorechr","sam_ignorechr","SAM/IGNORECHR",QVariant::String,"Which chromosome to ignore",QString(""));//chrM
}

/***************************************************************************************

****************************************************************************************/
QFileInfo Arguments::fileInfo(const QString& str)
{
    return QFileInfo(gArgs().getArgs(str).toString());
}

/***************************************************************************************

****************************************************************************************/
QStringList  Arguments::split(const QString& str,const QChar& sep)
{
    return gArgs().getArgs(str).toString().split(sep);
}

/***************************************************************************************

****************************************************************************************/
void Arguments::usage(void)
{
   cout << "Usage:" <<endl;
   foreach(const QString &key,getVarValStorage().keys())
	{
        if(getVarValStorage()[key]._cname!="")
        {
            cout<<"--"<<getVarValStorage()[key]._cname.toStdString()<<"\t\t\t"<<getVarValStorage()[key]._descr.toStdString()<<endl;
        }
    }
}

/***************************************************************************************

****************************************************************************************/
void Arguments::addArg(QString key,QString _c/*command line argument*/, QString _i/*name in ini file*/,QVariant::Type _t, QString _d,QVariant _def,bool _r/*is argument required or not*/)
{ 
    if(!getVarValStorage().contains(key))
        getVarValStorage().insert(key,_ArgDescr(_c,_i,_t,_def,_d,_r));	
}

/***************************************************************************************

****************************************************************************************/
QVariant &Arguments::getArgs(QString key)
{
    mutex.lock();
	if(getVarValStorage().contains(key))
	{
		if(getVarValStorage()[key]._value.isValid() && !getVarValStorage()[key]._value.isNull())
        {
            QVariant &_v=getVarValStorage()[key]._value;
            mutex.unlock();
            return _v;
        }
		if(getVarValStorage()[key]._defValue.isValid() && !getVarValStorage()[key]._defValue.isNull())
        {
            QVariant &_v=getVarValStorage()[key]._defValue;
            mutex.unlock();
			return _v;
        }
	}
   
    mutex.unlock();
	throw "Incorrect key";
}
/***************************************************************************************

****************************************************************************************/
