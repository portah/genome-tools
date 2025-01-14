/****************************************************************************
**
** Copyright (C) 2011 Andrey Kartashov .
** All rights reserved.
** Contact: Andrey Kartashov (porter@porter.st)
**
** This file is part of the ReadsCounting module of the genome-tools.
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
#ifndef _ReadsCounting_
#define _ReadsCounting_


#include <config.hpp>

struct Isoform;

typedef quint64 t_genome_coordinates;
typedef unsigned int t_reads_count;

typedef QSharedPointer<Isoform> IsoformPtr;
typedef QVector< IsoformPtr > refseq;
typedef QMap<QString, refseq> IsoformsOnChromosome;
//typedef bicl::interval_map<t_genome_coordinates,t_reads_count> chrom_coverage;

typedef bicl::interval_map<t_genome_coordinates,QList< IsoformPtr > > chrom_coverage;

struct Isoform
{
    QString name;
    QString name2;
    QString chrom;
    QChar   strand;
    quint64 txStart;
    quint64 txEnd;
    quint64 cdsStart;
    quint64 cdsEnd;
    int exCount;
    chrom_coverage isoform;//?
    /*QSharedPointer for storing shared isoforms reads, and storing isoforms intersections*/
    QSharedPointer<chrom_coverage > intersects_count;
    QSharedPointer<QList<IsoformPtr> > intersects_isoforms;/*Pointers to intersected chromosomes*/
    //QSharedPointer<chrom_coverage > general;/*common part for intersected isoforms plus reads*/
    quint64 len;
    double totReads;
    double RPKM;
    bool testNeeded;
    bool intersects;
    quint16 min;
    double density;
    quint16 count;

    Isoform():
    exCount(0),
    len(0),
    RPKM(0),
    min(-1),
    density(0.0),
    count(0){};

    Isoform(QString n,QString n2,QString chr,QChar s,quint64 txs,quint64 txe,quint64 cdss,quint64 cdse)://,chrom_coverage iso,quint64 l):
        name(n),
        name2(n2),
        chrom(chr),
        strand(s),
        txStart(txs),
        txEnd(txe),
        cdsStart(cdss),
        cdsEnd(cdse),
        //exCount(iso.iterative_size()),
        //isoform(iso),
        //len(l),
        totReads(0),
        RPKM(0),
        testNeeded(false),
        intersects(false),
        min(-1),
        density(0.0),
        count(0){};
};

template <class T>
bool operator<=(QList<T> &s, QList<T> &s1) {
 return s.size()<=s1.size();
}

template <class T>
bool operator>=(QList<T> &s, QList<T> &s1) {
 return s.size()>=s1.size();
}
#include <Reads.hpp>
typedef genome::GenomeDescription gen_lines;

//-------------------------------------------------------------------------------------------------------
#define FSTM ReadsCounting

class sam_reader_thread;

class FSTM: public QObject
{
    Q_OBJECT

private:
    int m_ThreadCount;
    int m_ThreadNum;
    QSqlQuery q;
    double totIsoLen;
    IsoformsOnChromosome** isoforms; //QVector<QVector<> > ???
    gen_lines** sam_data;
    //sam_reader_thread** threads;
    //QList<sam_reader_thread*>   t_queue;
    QVector<QMap<QString, QVector< IsoformPtr > > > TSS_organized_list;
    QVector<QMap<QString, QVector< IsoformPtr > > > GENES_organized_list;
    bool dUTP;
public:

    FSTM(QObject* parent=0);
    ~FSTM();

public slots:
    void start(void);

protected slots:

    void FillUpData(void);
    void WriteResult(void);
    void CreateTables(void);
    void InsertData(void);

signals:
    void finished(void);
};

#endif
