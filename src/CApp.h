#ifndef CAPP_H
#define CAPP_H
#include "../config.h"
#include "CSettings.h"
#include "CThread.h"

#include <QApplication>

class CGui;


class CAStream;
class QString;
class QOject;
class QEvent;
class CDetector;
class CNormalizer;
class QEvent;
class QKeyEvent;



class CApp: public QApplication
{

    Q_OBJECT
    public:
                        CApp        (int & argc, char ** argv);
        virtual        ~CApp        ();
        void            Init        ();

        CSettings       Settings;


        CAStream      * Play        ()                  const { return _Play;       }
        CAStream      * Rec         ()                  const { return _Rec;        }
        CGui          * Gui         ()                  const { return _Gui;        }
        CDetector     * Detector    ()                  const { return _Detector;   }
        CNormalizer   * Normalizer  ()                  const { return _Normalizer; }

#if 0
        void            Lock        ();
        void            Unlock      ();
#endif

        static  QString CleanString (QString const & in, bool keep_pathseps = false,
                                     bool keep_articles = false);
        static  void    USleep      (int microseconds);
        static  bool    CheckAccess (QString const & file_or_path, bool check_path_only,
                                     bool test_if_writable, bool throw_exception = true);

        static  QChar   SideToQChar (int side)            { return QChar ('a' + side); }
        static  int     QCharToSide (QChar side)          { return side.cell() - 'a'; }

        void            InitStreams (bool play, bool rec);


        bool            SetSettings (bool all_data_needed, bool read_settings_already_done);
        bool            OpenProject (QString const & proj_file);
        void            SaveProject ();
        bool            CloseProject(bool ask_changes);
        void            NewProject  (QString const & rebuild_dir = QString());



    private:
        void            change_uid   ();
        CGui*           _Gui;
        CAStream      * _Play;
        CAStream      * _Rec;
        CDetector     * _Detector;
        CNormalizer   * _Normalizer;
        bool            XRunDlg;
        volatile QMutex Mutex;

        void      keyPressEvent (QKeyEvent * event);

        void            rebuild_project(QString const & src_dir);
        static void     stream_callback(CAStream const * sender);


        bool            notify( QObject *o, QEvent *e );

    private slots:
        void            rec_xrun     ();


};

#endif

