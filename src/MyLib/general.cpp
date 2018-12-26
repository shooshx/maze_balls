
#include "general.h"

#include <QString>
#include <QCoreApplication>
#include <QMessageBox>


string humanCount(int64_t n)
{
	QString init = QString("%1").arg(n); 
	
	if (init.length() < 4) 
		return init.toLatin1().data();

	QByteArray buf = init.toLatin1();
	int len = buf.count();
	
	char rbuf[50] = {0};
	int ind = len - 1;
	int x = 0;
	for (int i = len + (len-1)/3 - 1; i >= 0; --i)
	{
		if (++x % 4 == 0) rbuf[i] = ',';
		else rbuf[i] = buf[ind--];
	}
	return rbuf;
}


int64_t powOf10[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, Q_INT64_C(1000000000), Q_INT64_C(10000000000), Q_INT64_C(100000000000) };

string humanCount(double n, int pers)
{
    int64_t dn = (qint64)n;
    int64_t pwr = powOf10[mMin(pers, 11)];
    int64_t fl = (qint64)((n - dn) * pwr);
	QString sfl = QString("%1").arg(fl).toLatin1();

	return humanCount(dn) + "." + sfl.toLatin1().data();

}

void flushAllEvents()
{
	QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents); // process the hide event. otherwise the 3d view is moved and it doesn't look nice.
	QCoreApplication::sendPostedEvents(nullptr, 0);
	QCoreApplication::flush();
}
