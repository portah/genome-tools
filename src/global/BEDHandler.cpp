/****************************************************************************
**
** Copyright (C) 2011 Andrey Kartashov .
** All rights reserved.
** Contact: Andrey Kartashov (porter@porter.st)
**
** This file is part of the global module of the genome-tools.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Andrey Kartashov.
**
****************************************************************************/

#include <BEDHandler.hpp>

//-------------------------------------------------------------
//-------------------------------------------------------------
BEDHandler::~BEDHandler() {
    if(!create_file)
        _outFile.close();
}

//-------------------------------------------------------------
//-------------------------------------------------------------
BEDHandler::BEDHandler(Storage& sam) {
    init(sam);
}

//-------------------------------------------------------------
//-------------------------------------------------------------
void BEDHandler::init(Storage& sam)
{
    sam_input=&sam;
    create_file=gArgs().getArgs("no-bed-file").toBool();
    bed_type=gArgs().getArgs("bed_type").toInt();
    window=gArgs().getArgs("bed_window").toUInt();
    no_sql_upload=gArgs().getArgs("no-sql-upload").toBool();

    aligned=((float)(sam_input->total-sam_input->notAligned))/1000000.0;
    normalize=gArgs().getArgs("bed_normalize").toBool();

    if(window<=0) window=0;

    if(!create_file) {
        _outFile.setFileName(gArgs().fileInfo("out").baseName()+".bed");
        _outFile.open(QIODevice::WriteOnly|QIODevice::Truncate);
        if(gArgs().getArgs("bed_HeaderString").toString().contains('%')) {
            _outFile.write((gArgs().getArgs("bed_HeaderString").toString().arg(_outFile.fileName())+"\n").toLocal8Bit());
            _outFile.flush();
        } else {
            _outFile.write((gArgs().getArgs("bed_HeaderString").toString()+"\n").toLocal8Bit());
            _outFile.flush();
        }
    }

    if(no_sql_upload) return;

    QSqlDatabase db = QSqlDatabase::database();
    db.close();
    if (!db.open() ) {
        QSqlError sqlErr = db.lastError();
        qDebug()<<qPrintable("Error connect to DB:"+sqlErr.text());
        throw "Error connect to DB";
    }

    if(!q.exec("DROP TABLE IF EXISTS "+gArgs().getArgs("sql_table").toString()+";")) {
        qWarning()<<qPrintable("Drop table query error. "+q.lastError().text());
    }


    switch(gArgs().getArgs("bed_format").toInt()) {
        case 4:
            if(!q.exec("CREATE TABLE "+gArgs().getArgs("sql_table").toString()+
                       "( "
                       "bin int(7) unsigned NOT NULL, "
                       "chrom varchar(255) NOT NULL, "
                       "chromStart int(10) unsigned NOT NULL, "
                       "chromEnd int(10) unsigned NOT NULL, "
                       "name varchar(255) NOT NULL, "
                       "INDEX bin_idx (bin) using btree,"
                       "INDEX chrom_idx (chrom) using btree,"
                       "INDEX chrom_start_idx (chromStart) using btree,"
                       "INDEX chrom_end_idx (chromEnd) using btree"
                       ") ENGINE=MyISAM DEFAULT CHARSET=utf8"))
            {
                qWarning()<<qPrintable("Create table error. "+q.lastError().text());
                exit(-1);
            }
            sql_prep="START TRANSACTION; INSERT INTO "+gArgs().getArgs("sql_table").toString()+" (bin,chrom,chromStart,chromEnd,name) VALUES";
        break;
        case 8:
            if(!q.exec("CREATE TABLE "+gArgs().getArgs("sql_table").toString()+
                       "( "
                       "bin int(7) unsigned NOT NULL,"
                       "chrom varchar(255) NOT NULL,"
                       "chromStart int(10) unsigned NOT NULL,"
                       "chromEnd int(10) unsigned NOT NULL,"
                       "name varchar(255) NOT NULL,"
                       "score int(5) not null,"
                       "strand char not null,"
                       "INDEX bin_idx (bin) using btree,"
                       "INDEX chrom_idx (chrom) using btree,"
                       "INDEX chrom_start_idx (chromStart) using btree,"
                       "INDEX chrom_end_idx (chromEnd) using btree"
                       ") ENGINE=MyISAM DEFAULT CHARSET=utf8"))
            {
                qWarning()<<qPrintable("Create 1 query error. "+q.lastError().text());
            }
            sql_prep="START TRANSACTION; INSERT INTO "+gArgs().getArgs("sql_table").toString()+" (bin,chrom,chromStart,chromEnd,name,score,strand) VALUES";
        break;
    }

}

//-------------------------------------------------------------
//-------------------------------------------------------------
void BEDHandler::Load()
{
    quint32 shift= gArgs().getArgs("bed_siteshift").toUInt();

    foreach(QString chrom,sam_input->getLines()) {

        if(chrom.endsWith("-")) continue;
        chrom.chop(1);
        QMap <int,int> bed;
        QList<int> cover;

        if(bed_type==2 || bed_type==3) {//covers
            cover.reserve(sam_input->getLength('+',chrom)+1);
            for(unsigned int cc=0;cc<sam_input->getLength('+',chrom)+1;cc++)
                cover.append(0);
        }

        //+ strand
        fill_bed_cover(bed,cover,chrom,'+',shift);

        if(gArgs().getArgs("bed_format").toInt() == 8) {
            if(bed_type==0) {
                bed_save(bed,sql_prep,chrom,'+');
                bed.clear();
            }
            if(bed_type==2 || bed_type==3) {
                cover_save(cover,sql_prep,chrom, '+');
                for(unsigned int cc=0;cc<sam_input->getLength('+',chrom)+1;cc++)
                    cover[cc]=0;
            }
        }

        //- strand
        fill_bed_cover(bed,cover,chrom,'-',-shift);
        if(bed_type==0 || bed_type==1) {
            bed_save(bed,sql_prep,chrom,'-');
        }
        if(bed_type==2 || bed_type==3) {
            cover_save(cover,sql_prep,chrom, '-');
        }
        qDebug()<<"Complete chrom:"<<chrom;
    }//foreach

    if(!create_file) {
        _outFile.flush();
    }
}//end of function


#define MAX_GRP 3000
//-------------------------------------------------------------
//-------------------------------------------------------------
void BEDHandler::cover_save(QList<int>& cover,QString& sql_prep,QString const& chrom, QChar const& strand) {
    QString appe;
    quint64 begin=0;
    int binw=aligned/10+2;
    int bins=0;
    int sql_groupping=0;
    int old=0;
    float valm=0;

    binw=(binw>10)?10:binw;
    if(!gArgs().getArgs("bed_autoresolution").toBool())
        binw=1;
    switch(gArgs().getArgs("bed_format").toInt()) {
        case 4:
            for(qint64 i=0;i<cover.size();i++) {
                if(cover[i] == old) {bins++; continue;}
                if(old == 0) { old=cover[i]; begin=i; bins=1; continue; }

                float val;
                if(normalize) {
                    val=old/aligned;
                } else {
                    val=old;
                }
                valm=qMax(val,valm);

                if(bins<binw && cover[i]!=0) {
                    bins++;
                    old=cover[i];
                    continue;
                }
                if(!create_file)
                    _outFile.write(QString(chrom+"\t%1\t%2\t%3\n").arg(begin-1).arg(i-1).arg(val).toLocal8Bit());

                if(!no_sql_upload) {
                    sql_groupping++;

                    appe+=QString(" (%1,'%2',%3,%4,%5),").arg(binFromRange(begin-1,i-1)).arg(chrom).arg(begin-1).arg(i-1).arg(val);
                    if(sql_groupping==MAX_GRP) {
                        sql_groupping=0;
                        appe.chop(1);
                        if(!q.exec(sql_prep+appe+"; COMMIT;"))
                            qWarning()<<qPrintable("COMMIT query error (cover 1). "+q.lastError().text());
                        appe.clear();
                    }
                }
                begin=i;
                old=cover[i];
                bins=0;
            }
        break;
        case 8:
            for(qint64 i=0;i<cover.size();i++) {
                if(cover[i] == old) {bins++; continue;}
                if(old == 0) { old=cover[i]; begin=i; bins=0; continue; }

                float val;
                if(normalize) {
                    val=old/aligned;
                } else {
                    val=old;
                }

                valm=qMax(val,valm);

                if(bins<binw && cover[i]!=0) {
                    bins++;
                    old=cover[i];
                    continue;
                }

                if(!create_file)
                    _outFile.write(QString(chrom+"\t%1\t%2\t"+"+strand+"+"%3\t0\t"+strand+"\n").arg(begin-1).arg(i-1).arg(val).toLocal8Bit());

                if(!no_sql_upload) {
                    sql_groupping++;
                    appe+=QString(" (%1,'%2',%3,%4,%5%6,0,'%7'),").arg(binFromRange(begin-1,i-1)).arg(chrom).arg(begin-1).arg(i-1).arg(strand).arg(val).arg(strand);
                    if(sql_groupping==MAX_GRP) {
                        sql_groupping=0;
                        appe.chop(1);
                        if(!q.exec(sql_prep+appe+"; COMMIT;"))
                            qWarning()<<qPrintable("COMMIT query error (cover 2). "+q.lastError().text());
                        appe.clear();
                    }
                }
                begin=i;
                old=cover[i];
                bins=0;
            }
        break;
    }//switch
    if(!no_sql_upload && sql_groupping>0) {
        appe.chop(1);
        if(!q.exec(sql_prep+appe+"; COMMIT;"))
            qWarning()<<qPrintable("COMMIT query error (cover 3). "+q.lastError().text());
    }
}

//-------------------------------------------------------------
//-------------------------------------------------------------

void BEDHandler::bed_save(QMap <int,int>& bed,QString& sql_prep,QString const& chrom, QChar const& strand) {

    //QSqlDatabase db = QSqlDatabase::database();
    QString appe;
    int sql_groupping=0;
    QMap<int,int>::iterator i = bed.begin();


    switch(gArgs().getArgs("bed_format").toInt()) {
        case 4:
            for(;i!=bed.end();i++) {

                if(!create_file)
                    _outFile.write(QString(chrom+"\t%1\t%2\t%3\n").arg(i.key()).arg(i.key()+window).arg(i.value()).toLocal8Bit());

                if(!no_sql_upload) {
                    sql_groupping++;
                    float val;
                    if(normalize) {
                        val=i.value()/aligned;
                    } else {
                        val=i.value();
                    }
                    appe+=QString(" (0,'%1',%2,%3,%4),").arg(chrom).arg(i.key()).arg(i.key()+window).arg(val);
                    if(sql_groupping==MAX_GRP) {
                        sql_groupping=0;
                        appe.chop(1);
                        if(!q.exec(sql_prep+appe+"; COMMIT;"))
                            qWarning()<<qPrintable("COMMIT query error (bed 1). "+q.lastError().text());
                        appe.clear();
                    }
                }
            }
        break;
        case 8:
            //-----------------------------------------------------------------------------------

            for(;i!=bed.end();i++) {

                if(!create_file)
                    _outFile.write(QString(chrom+"\t%1\t%2\t"+strand+"%3\t0\t"+strand+"\n").arg(i.key()).arg(i.key()+window).arg(i.value()).toLocal8Bit());

                if(!no_sql_upload) {
                    sql_groupping++;
                    float val;
                    if(normalize) {
                        val=i.value()/aligned;
                    } else {
                        val=i.value();
                    }
                    appe+=QString(" (0,'%1',%2,%3,%4%5,0,'%6'),").arg(chrom).arg(i.key()).arg(i.key()+window).arg(strand).arg(val).arg(strand);
                    if(sql_groupping==MAX_GRP) {
                        sql_groupping=0;
                        appe.chop(1);
                        if(!q.exec(sql_prep+appe+"; COMMIT;"))
                            qWarning()<<qPrintable("COMMIT query error (bed 2). "+q.lastError().text());
                        appe.clear();
                    }
                }
            }
        break;
    }//switch

    if(!no_sql_upload && appe.length()>0) {
        appe.chop(1);
        if(!q.exec(sql_prep+appe+"; COMMIT;"))
            qWarning()<<qPrintable("COMMIT query error (bed 3). "+q.lastError().text());
    }
}


//-------------------------------------------------------------
//-------------------------------------------------------------
void BEDHandler::fill_bed_cover(QMap <int,int>& bed,QList<int>& cover,QString const& chrom,QChar const& strand,int shift) {

    genome::cover_map::iterator i=sam_input->getLineCover(chrom+strand).getBeginIterator();
    genome::cover_map::iterator e=sam_input->getLineCover(chrom+strand).getEndIterator();
    int max_len=sam_input->getLength(strand,chrom);
    bool direction=(strand==QChar('+'));

    quint32 w_h=window;
    if(window==0) w_h=1;

    switch(bed_type) {
        case 0:
            for(;i!=e;i++) {
                int val=i.key()+shift;
                int index=val-val%w_h;
                genome::Cover::countReads<int>(i.value(),bed[qMin(qMax(0,index),max_len)]);
            }
        break;
        case 1:
            for(;i!=e;i++)
                for(int c=0;c<i.value().size();c++) {
                    int val=i.key()+i.value()[c].getLength()/2+shift;
                    int index=val-val%w_h;
                    bed[qMin(qMax(0,index),max_len)]+=i.value()[c].getLevel();
                }
        break;
        case 2:
            for(;i!=e;i++)//through start positions
                for(int c=0;c<i.value().size();c++) {//thru different reads at the same position
                    genome::read_representation::const_iterator it=i.value()[c].getInterval().begin();
                    for(;it!=i.value()[c].getInterval().end();it++) {
                        genome::read_representation::interval_type itv  = bicl::key_value<genome::read_representation>(it);
                        for(quint64 l=itv.lower(); l<=itv.upper(); l++)
                            cover[l]+=i.value()[c].getLevel();
                    }
                }
        break;
        case 3:
            qint64 interestedLen=window;
            for(;i!=e;i++)//through start positions
                for(int c=0;c<i.value().size();c++) {//thru different reads at the same position
                    genome::read_representation::const_iterator it=i.value()[c].getInterval().begin();
                    for(;it!=i.value()[c].getInterval().end();it++) {
                        genome::read_representation::interval_type itv  = bicl::key_value<genome::read_representation>(it);
                        qint64 beg=itv.lower();
                        qint64 end=itv.upper();
                        if(direction) {
                            end=beg+interestedLen;
                        } else {
                            beg=end-interestedLen;
                        }

                        if(beg<0) beg=0;
                        if(end>max_len) end=max_len;

                        for(qint64 l=beg; l<=end; l++)
                            cover[l]+=i.value()[c].getLevel();
                    }
                }
        break;
    }//switch
}
