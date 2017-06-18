#ifndef D_sdachiBuildTime_H
#define D_sdachiBuildTime_H

///////////////////////////////////////////////////////////////////////////////
//
//  sdachiBuildTime is responsible for recording and reporting when
//  this project library was built
//
///////////////////////////////////////////////////////////////////////////////

class sdachiBuildTime
  {
  public:
    explicit sdachiBuildTime();
    virtual ~sdachiBuildTime();
    
    const char* GetDateTime();

  private:
      
    const char* dateTime;

    sdachiBuildTime(const sdachiBuildTime&);
    sdachiBuildTime& operator=(const sdachiBuildTime&);

  };

#endif  // D_sdachiBuildTime_H
