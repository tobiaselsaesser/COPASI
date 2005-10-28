/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/CopasiUI/Attic/CQValidator.h,v $
   $Revision: 1.3 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/10/28 17:56:23 $
   End CVS Header */

#ifndef COPASI_CQValidator
 #define COPASI_CQValidator

#include <qvalidator.h>
 #include <qcolor.h>
 #include <qlineedit.h>

class CQValidator : public QValidator
  {
    // Operations
  public:
    CQValidator(QLineEdit * parent, const char * name = 0);

    virtual State validate(QString & input, int & pos) const;

    virtual State revalidate();

    virtual void force(const QString & input) const;

    virtual void saved() const;

  protected:
    State setColor(const State & state) const;

    //Attributes
    QLineEdit * mpLineEdit;

  private:
    QString mLastAccepted;

    QColor mSavedColor;

    QColor mAcceptableColor;

    QColor mErrorColor;
  };

class CQValidatorNotEmpty : public CQValidator
  {
    // Operations
  public:
    CQValidatorNotEmpty(QLineEdit * parent, const char * name = 0);

    virtual State validate(QString & input, int & pos) const;
  };

class CQValidatorBound : public CQValidator
  {
    // Operations
  public:
    CQValidatorBound(QLineEdit * parent, const char * name = 0);

    virtual State validate(QString & input, int & pos) const;

    virtual void force(const QString & input) const;

    //Attributes
  protected:
    QDoubleValidator * mpDoubleValidator;

    QString mValidBound;
  };

#endif // COPASI_CQValidator
