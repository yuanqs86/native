/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2011 Live Networks, Inc.  All rights reserved.
// A generic RTSP client
// Implementation

#include "RTSPClient.hh"
#include "RTSPCommon.hh"
#include "Base64.hh"
#include "Locale.hh"
#include <GroupsockHelper.hh>
#include "our_md5.h"

////////// RTSPClient implementation //////////

RTSPClient* RTSPClient::createNew(UsageEnvironment& env, char const* rtspURL,
				  int verbosityLevel,
				  char const* applicationName,
				  portNumBits tunnelOverHTTPPortNum) {
  return new RTSPClient(env, rtspURL,
			verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

unsigned RTSPClient::sendDescribeCommand(responseHandler* responseHandler, Authenticator* authenticator) {
  if (authenticator != NULL) fCurrentAuthenticator = *authenticator;
  return sendRequest(new RequestRecord(++fCSeq, "DESCRIBE", responseHandler));
}

unsigned RTSPClient::sendOptionsCommand(responseHandler* responseHandler, Authenticator* authenticator) {
  if (authenticator != NULL) fCurrentAuthenticator = *authenticator;
  return sendRequest(new RequestRecord(++fCSeq, "OPTIONS", responseHandler));
}

unsigned RTSPClient::sendAnnounceCommand(char const* sdpDescription, responseHandler* responseHandler, Authenticator* authenticator) {
  if (authenticator != NULL) fCurrentAuthenticator = *authenticator;
  return sendRequest(new RequestRecord(++fCSeq, "ANNOUNCE", responseHandler, NULL, NULL, False, 0.0, 0.0, 0.0, sdpDescription));
}

unsigned RTSPClient::sendSetupCommand(MediaSubsession& subsession, responseHandler* responseHandler,
                                      Boolean streamOutgoing, Boolean streamUsingTCP, Boolean forceMulticastOnUnspecified,
				      Authenticator* authenticator) {
  if (fTunnelOverHTTPPortNum != 0) streamUsingTCP = True; // RTSP-over-HTTP tunneling uses TCP (by definition)
  if (authenticator != NULL) fCurrentAuthenticator = *authenticator;

  u_int32_t booleanFlags = 0;
  if (streamUsingTCP) booleanFlags |= 0x1;
  if (streamOutgoing) booleanFlags |= 0x2;
  if (forceMulticastOnUnspecified) booleanFlags |= 0x4;
  return sendRequest(new RequestRecord(++fCSeq, "SETUP", responseHandler, NULL, &subsession, booleanFlags));
}

unsigned RTSPClient::sendPlayCommand(MediaSession& session, responseHandler* responseHandler,
                                     double start, double end, float scale,
                                     Authenticator* authenticator) {
  if (authenticator != NULL) fCurrentAuthenticator = *authenticator;
  return sendRequest(new RequestRecord(++fCSeq, "PLAY", responseHandler, &session, NULL, 0, start, end, scale));
}

unsigned RTSPClient::sendPlayCommand(MediaSubsession& subsession, responseHandler* responseHandler,
                                     double start, double end, float scale,
                                     Authenticator* authenticator) {
  if (authenticator != NULL) fCurrentAuthenticator = *authenticator;
  return sendRequest(new RequestRecord(++fCSeq, "PLAY", responseHandler, NULL, &subsession, 0, start, end, scale));
}

unsigned RTSPClient::sendPauseCommand(MediaSession& session, responseHandler* responseHandler, Authenticator* authenticator) {
  if (authenticator != NULL) fCurrentAuthenticator = *authenticator;
  return sendRequest(new RequestRecord(++fCSeq, "PAUSE", responseHandler, &session));
}

unsigned RTSPClient::sendPauseCommand(MediaSubsession& subsession, responseHandler* responseHandler, Authenticator* authenticator) {
  if (authenticator != NULL) fCurrentAuthenticator = *authenticator;
  return sendRequest(new RequestRecord(++fCSeq, "PAUSE", responseHandler, NULL, &subsession));
}

unsigned RTSPClient::sendRecordCommand(MediaSession& session, responseHandler* responseHandler, Authenticator* authenticator) {
  if (authenticator != NULL) fCurrentAuthenticator = *authenticator;
  return sendRequest(new RequestRecord(++fCSeq, "RECORD", responseHandler, &session));
}

unsigned RTSPClient::sendRecordCommand(MediaSubsession& subsession, responseHandler* responseHandler, Authenticator* authenticator) {
  if (authenticator != NULL) fCurrentAuthenticator = *authenticator;
  return sendRequest(new RequestRecord(++fCSeq, "RECORD", responseHandler, NULL, &subsession));
}

unsigned RTSPClient::sendTeardownCommand(MediaSession& session, responseHandler* responseHandler, Authenticator* authenticator) {
  if (authenticator != NULL) fCurrentAuthenticator = *authenticator;
  return sendRequest(new RequestRecord(++fCSeq, "TEARDOWN", responseHandler, &session));
}

unsigned RTSPClient::sendTeardownCommand(MediaSubsession& subsession, responseHandler* responseHandler, Authenticator* authenticator) {
  if (authenticator != NULL) fCurrentAuthenticator = *authenticator;
  return sendRequest(new RequestRecord(++fCSeq, "TEARDOWN", responseHandler, NULL, &subsession));
}

unsigned RTSPClient::sendSetParameterCommand(MediaSession& session, responseHandler* responseHandler,
                                             char const* parameterName, char const* parameterValue,
                                             Authenticator* authenticator) {
  if (authenticator != NULL) fCurrentAuthenticator = *authenticator;
  char* paramString = new char[strlen(parameterName) + strlen(parameterValue) + 10];
  sprintf(paramString, "%s: %s\r\n", parameterName, parameterValue);
  unsigned result = sendRequest(new RequestRecord(++fCSeq, "SET_PARAMETER", responseHandler, &session, NULL, False, 0.0, 0.0, 0.0, paramString));
  delete[] paramString;
  return result;
}

unsigned RTSPClient::sendGetParameterCommand(MediaSession& session, responseHandler* responseHandler, char const* parameterName,
                                             Authenticator* authenticator) {
  if (authenticator != NULL) fCurrentAuthenticator = *authenticator;

  // We assume that:
  //    parameterName is NULL means: Send no body in the request.
  //    parameterName is "" means: Send only \r\n in the request body.
  //    parameterName is non-empty means: Send "<parameterName>\r\n" as the request body.
  unsigned parameterNameLen = parameterName == NULL ? 0 : strlen(parameterName);
  char* paramString = new char[parameterNameLen + 3]; // the 3 is for \r\n + the '\0' byte
  if (parameterName == NULL) {
    paramString[0] = '\0';
  } else {
    sprintf(paramString, "%s\r\n", parameterName);
  }
  unsigned result = sendRequest(new RequestRecord(++fCSeq, "GET_PARAMETER", responseHandler, &session, NULL, False, 0.0, 0.0, 0.0, paramString));
  delete[] paramString;
  return result;
}

Boolean RTSPClient::changeResponseHandler(unsigned cseq, responseHandler* newResponseHandler) {
  // Look for the matching request record in each of our 'pending requests' queues:
  RequestRecord* request;
  if ((request = fRequestsAwaitingConnection.findByCSeq(cseq)) != NULL
      || (request = fRequestsAwaitingHTTPTunneling.findByCSeq(cseq)) != NULL
      || (request = fRequestsAwaitingResponse.findByCSeq(cseq)) != NULL) {
    request->handler() = newResponseHandler;
    return True;
  }

  return False;
}

Boolean RTSPClient::lookupByName(UsageEnvironment& env,
				 char const* instanceName,
				 RTSPClient*& resultClient) {
  resultClient = NULL; // unless we succeed

  Medium* medium;
  if (!Medium::lookupByName(env, instanceName, medium)) return False;

  if (!medium->isRTSPClient()) {
    env.setResultMsg(instanceName, " is not a RTSP client");
    return False;
  }

  resultClient = (RTSPClient*)medium;
  return True;
}

Boolean RTSPClient::parseRTSPURL(UsageEnvironment& env, char const* url,
				 char*& username, char*& password,
				 NetAddress& address,
				 portNumBits& portNum,
				 char const** urlSuffix) {
  do {
    // Parse the URL as "rtsp://[<username>[:<password>]@]<server-address-or-name>[:<port>][/<stream-name>]"
    char const* prefix = "rtsp://";
    unsigned const prefixLength = 7;
    if (_strncasecmp(url, prefix, prefixLength) != 0) {
      env.setResultMsg("URL is not of the form \"", prefix, "\"");
      break;
    }

    unsigned const parseBufferSize = 100;
    char parseBuffer[parseBufferSize];
    char const* from = &url[prefixLength];

    // Check whether "<username>[:<password>]@" occurs next.
    // We do this by checking whether '@' appears before the end of the URL, or before the first '/'.
    username = password = NULL; // default return values
    char const* colonPasswordStart = NULL;
    char const* p;
    for (p = from; *p != '\0' && *p != '/'; ++p) {
      if (*p == ':' && colonPasswordStart == NULL) {
	colonPasswordStart = p;
      } else if (*p == '@') {
	// We found <username> (and perhaps <password>).  Copy them into newly-allocated result strings:
	if (colonPasswordStart == NULL) colonPasswordStart = p;

	char const* usernameStart = from;
	unsigned usernameLen = colonPasswordStart - usernameStart;
	username = new char[usernameLen + 1] ; // allow for the trailing '\0'
	for (unsigned i = 0; i < usernameLen; ++i) username[i] = usernameStart[i];
	username[usernameLen] = '\0';

	char const* passwordStart = colonPasswordStart;
	if (passwordStart < p) ++passwordStart; // skip over the ':'
	unsigned passwordLen = p - passwordStart;
	password = new char[passwordLen + 1]; // allow for the trailing '\0'
	for (unsigned j = 0; j < passwordLen; ++j) password[j] = passwordStart[j];
	password[passwordLen] = '\0';

	from = p + 1; // skip over the '@'
	break;
      }
    }

    // Next, parse <server-address-or-name>
    char* to = &parseBuffer[0];
    unsigned i;
    for (i = 0; i < parseBufferSize; ++i) {
      if (*from == '\0' || *from == ':' || *from == '/') {
	// We've completed parsing the address
	*to = '\0';
	break;
      }
      *to++ = *from++;
    }
    if (i == parseBufferSize) {
      env.setResultMsg("URL is too long");
      break;
    }

    NetAddressList addresses(parseBuffer);
    if (addresses.numAddresses() == 0) {
      env.setResultMsg("Failed to find network address for \"",
		       parseBuffer, "\"");
      break;
    }
    address = *(addresses.firstAddress());

    portNum = 554; // default value
    char nextChar = *from;
    if (nextChar == ':') {
      int portNumInt;
      if (sscanf(++from, "%d", &portNumInt) != 1) {
	env.setResultMsg("No port number follows ':'");
	break;
      }
      if (portNumInt < 1 || portNumInt > 65535) {
	env.setResultMsg("Bad port number");
	break;
      }
      portNum = (portNumBits)portNumInt;
      while (*from >= '0' && *from <= '9') ++from; // skip over port number
    }

    // The remainder of the URL is the suffix:
    if (urlSuffix != NULL) *urlSuffix = from;

    return True;
  } while (0);

  return False;
}

void RTSPClient::setUserAgentString(char const* userAgentName) {
  if (userAgentName == NULL) return;

  // Change the existing user agent header string:
  char const* const formatStr = "User-Agent: %s\r\n";
  unsigned const headerSize = strlen(formatStr) + strlen(userAgentName);
  delete[] fUserAgentHeaderStr;
  fUserAgentHeaderStr = new char[headerSize];
  sprintf(fUserAgentHeaderStr, formatStr, userAgentName);
  fUserAgentHeaderStrLen = strlen(fUserAgentHeaderStr);
}

unsigned RTSPClient::responseBufferSize = 20000; // default value; you can reassign this in your application if you need to

RTSPClient::RTSPClient(UsageEnvironment& env, char const* rtspURL,
		       int verbosityLevel, char const* applicationName,
		       portNumBits tunnelOverHTTPPortNum)
  : Medium(env),
    fVerbosityLevel(verbosityLevel), fTunnelOverHTTPPortNum(tunnelOverHTTPPortNum),
    fUserAgentHeaderStr(NULL), fUserAgentHeaderStrLen(0), fInputSocketNum(-1), fOutputSocketNum(-1), fServerAddress(0), fCSeq(1),
    fBaseURL(NULL), fTCPStreamIdCount(0), fLastSessionId(NULL), fSessionTimeoutParameter(0),
    fSessionCookieCounter(0), fHTTPTunnelingConnectionIsPending(False) {
  setBaseURL(rtspURL);

  fResponseBuffer = new char[responseBufferSize+1];
  resetResponseBuffer();

  // Set the "User-Agent:" header to use in each request:
  char const* const libName = "LIVE555 Streaming Media v";
  char const* const libVersionStr = LIVEMEDIA_LIBRARY_VERSION_STRING;
  char const* libPrefix; char const* libSuffix;
  if (applicationName == NULL || applicationName[0] == '\0') {
    applicationName = libPrefix = libSuffix = "";
  } else {
    libPrefix = " (";
    libSuffix = ")";
  }
  unsigned userAgentNameSize
    = strlen(applicationName) + strlen(libPrefix) + strlen(libName) + strlen(libVersionStr) + strlen(libSuffix) + 1;
  char* userAgentName = new char[userAgentNameSize];
  sprintf(userAgentName, "%s%s%s%s%s", applicationName, libPrefix, libName, libVersionStr, libSuffix);
  setUserAgentString(userAgentName);
  delete[] userAgentName;
}

RTSPClient::~RTSPClient() {
  reset();

  delete[] fResponseBuffer;
  delete[] fUserAgentHeaderStr;
}

Boolean RTSPClient::isRTSPClient() const {
  return True;
}

void RTSPClient::reset() {
  resetTCPSockets();
  resetResponseBuffer();
  fServerAddress = 0;

  setBaseURL(NULL);

  fCurrentAuthenticator.reset();

  delete[] fLastSessionId; fLastSessionId = NULL;
}

void RTSPClient::resetTCPSockets() {
  if (fInputSocketNum >= 0) {
    envir().taskScheduler().disableBackgroundHandling(fInputSocketNum);
    ::closeSocket(fInputSocketNum);
    if (fOutputSocketNum != fInputSocketNum) {
      envir().taskScheduler().disableBackgroundHandling(fOutputSocketNum);
      ::closeSocket(fOutputSocketNum);
    }
  }
  fInputSocketNum = fOutputSocketNum = -1;
}

void RTSPClient::resetResponseBuffer() {
  fResponseBytesAlreadySeen = 0;
  fResponseBufferBytesLeft = responseBufferSize;
}

void RTSPClient::setBaseURL(char const* url) {
  delete[] fBaseURL; fBaseURL = strDup(url);
}

int RTSPClient::openConnection() {
  do {
    // Set up a connection to the server.  Begin by parsing the URL:

    char* username;
    char* password;
    NetAddress destAddress;
    portNumBits urlPortNum;
    char const* urlSuffix;
    if (!parseRTSPURL(envir(), fBaseURL, username, password, destAddress, urlPortNum, &urlSuffix)) break;
    portNumBits destPortNum = fTunnelOverHTTPPortNum == 0 ? urlPortNum : fTunnelOverHTTPPortNum;
    if (username != NULL || password != NULL) {
      fCurrentAuthenticator.setUsernameAndPassword(username, password);
      delete[] username;
      delete[] password;
    }

    // We don't yet have a TCP socket (or we used to have one, but it got closed).  Set it up now.
    fInputSocketNum = fOutputSocketNum = setupStreamSocket(envir(), 0);
    if (fInputSocketNum < 0) break;

    // Connect to the remote endpoint:
    fServerAddress = *(netAddressBits*)(destAddress.data());
    int connectResult = connectToServer(fInputSocketNum, destPortNum);
    if (connectResult < 0) break;
    else if (connectResult > 0) {
      // The connection succeeded.  Arrange to handle responses to requests sent on it:
      envir().taskScheduler().setBackgroundHandling(fInputSocketNum, SOCKET_READABLE,
						    (TaskScheduler::BackgroundHandlerProc*)&incomingDataHandler, this);
    }
    return connectResult;
  } while (0);

  resetTCPSockets();
  return -1;
}

int RTSPClient::connectToServer(int socketNum, portNumBits remotePortNum) {
  MAKE_SOCKADDR_IN(remoteName, fServerAddress, htons(remotePortNum));
  if (fVerbosityLevel >= 1) {
    envir() << "Opening connection to " << AddressString(remoteName).val() << ", port " << remotePortNum << "...\n";
  }
  if (connect(socketNum, (struct sockaddr*) &remoteName, sizeof remoteName) != 0) {
    if (envir().getErrno() == EINPROGRESS) {
      // The connection is pending; we'll need to handle it later.  Wait for our socket to be 'writable', or have an exception.
      envir().taskScheduler().setBackgroundHandling(socketNum, SOCKET_WRITABLE|SOCKET_EXCEPTION,
						    (TaskScheduler::BackgroundHandlerProc*)&connectionHandler, this);
      return 0;
    }
    envir().setResultErrMsg("connect() failed: ");
    if (fVerbosityLevel >= 1) envir() << "..." << envir().getResultMsg() << "\n";
    return -1;
  }
  if (fVerbosityLevel >= 1) envir() << "...local connection opened\n";

  return 1;
}

char*
RTSPClient::createAuthenticatorString(char const* cmd, char const* url) {
  Authenticator& auth = fCurrentAuthenticator; // alias, for brevity
  if (auth.realm() != NULL && auth.username() != NULL && auth.password() != NULL) {
    // We have a filled-in authenticator, so use it:
    char* authenticatorStr;
    if (auth.nonce() != NULL) { // Digest authentication
      char const* const authFmt =
	"Authorization: Digest username=\"%s\", realm=\"%s\", "
	"nonce=\"%s\", uri=\"%s\", response=\"%s\"\r\n";
      char const* response = auth.computeDigestResponse(cmd, url);
      unsigned authBufSize = strlen(authFmt)
	+ strlen(auth.username()) + strlen(auth.realm())
	+ strlen(auth.nonce()) + strlen(url) + strlen(response);
      authenticatorStr = new char[authBufSize];
      sprintf(authenticatorStr, authFmt,
	      auth.username(), auth.realm(),
	      auth.nonce(), url, response);
      auth.reclaimDigestResponse(response);
    } else { // Basic authentication
      char const* const authFmt = "Authorization: Basic %s\r\n";

      unsigned usernamePasswordLength = strlen(auth.username()) + 1 + strlen(auth.password());
      char* usernamePassword = new char[usernamePasswordLength+1];
      sprintf(usernamePassword, "%s:%s", auth.username(), auth.password());

      char* response = base64Encode(usernamePassword, usernamePasswordLength);
      unsigned const authBufSize = strlen(authFmt) + strlen(response) + 1;
      authenticatorStr = new char[authBufSize];
      sprintf(authenticatorStr, authFmt, response);
      delete[] response; delete[] usernamePassword;
    }

    return authenticatorStr;
  }

  // We don't have a (filled-in) authenticator.
  return strDup("");
}

static char* createSessionString(char const* sessionId) {
  char* sessionStr;
  if (sessionId != NULL) {
    sessionStr = new char[20+strlen(sessionId)];
    sprintf(sessionStr, "Session: %s\r\n", sessionId);
  } else {
    sessionStr = strDup("");
  }
  return sessionStr;
}

static char* createScaleString(float scale, float currentScale) {
  char buf[100];
  if (scale == 1.0f && currentScale == 1.0f) {
    // This is the default value; we don't need a "Scale:" header:
    buf[0] = '\0';
  } else {
    Locale l("C", Numeric);
    sprintf(buf, "Scale: %f\r\n", scale);
  }

  return strDup(buf);
}

static char* createRangeString(double start, double end) {
  char buf[100];
  if (start < 0) {
    // We're resuming from a PAUSE; there's no "Range:" header at all
    buf[0] = '\0';
  } else if (end < 0) {
    // There's no end time:
    Locale l("C", Numeric);
    sprintf(buf, "Range: npt=%.3f-\r\n", start);
  } else {
    // There's both a start and an end time; include them both in the "Range:" hdr
    Locale l("C", Numeric);
    sprintf(buf, "Range: npt=%.3f-%.3f\r\n", start, end);
  }

  return strDup(buf);
}

unsigned RTSPClient::sendRequest(RequestRecord* request) {
  char* cmd = NULL;
  do {
    Boolean connectionIsPending = False;
    if (!fRequestsAwaitingConnection.isEmpty()) {
      // A connection is currently pending (with at least one enqueued request).  Enqueue this request also:
      connectionIsPending = True;
    } else if (fInputSocketNum < 0) { // we need to open a connection
      int connectResult = openConnection();
      if (connectResult < 0) break; // an error occurred
      else if (connectResult == 0) {
	// A connection is pending
        connectionIsPending = True;
      } // else the connection succeeded.  Continue sending the command.u
    }
    if (connectionIsPending) {
      fRequestsAwaitingConnection.enqueue(request);
      return request->cseq();
    }

    // If requested (and we're not already doing it, or have done it), set up the special protocol for tunneling RTSP-over-HTTP:
    if (fTunnelOverHTTPPortNum != 0 && strcmp(request->commandName(), "GET") != 0 && fOutputSocketNum == fInputSocketNum) {
      if (!setupHTTPTunneling1()) break;
      fRequestsAwaitingHTTPTunneling.enqueue(request);
      return request->cseq();
    }

    // Construct and send the command:

    // First, construct command-specific headers that we need:

    char* cmdURL = fBaseURL; // by default
    Boolean cmdURLWasAllocated = False;

    char const* protocolStr = "RTSP/1.0"; // by default

    char* extraHeaders = (char*)""; // by default
    Boolean extraHeadersWereAllocated = False;

    char* contentLengthHeader = (char*)""; // by default
    Boolean contentLengthHeaderWasAllocated = False;

    char const* contentStr = request->contentStr(); // by default
    if (contentStr == NULL) contentStr = "";
    unsigned contentStrLen = strlen(contentStr);
    if (contentStrLen > 0) {
      char const* contentLengthHeaderFmt =
	"Content-Length: %d\r\n";
      unsigned contentLengthHeaderSize = strlen(contentLengthHeaderFmt)
	+ 20 /* max int len */;
      contentLengthHeader = new char[contentLengthHeaderSize];
      sprintf(contentLengthHeader, contentLengthHeaderFmt, contentStrLen);
      contentLengthHeaderWasAllocated = True;
    }

    if (strcmp(request->commandName(), "DESCRIBE") == 0) {
      extraHeaders = (char*)"Accept: application/sdp\r\n";
    } else if (strcmp(request->commandName(), "OPTIONS") == 0) {
    } else if (strcmp(request->commandName(), "ANNOUNCE") == 0) {
      extraHeaders = (char*)"Content-Type: application/sdp\r\n";
    } else if (strcmp(request->commandName(), "SETUP") == 0) {
      MediaSubsession& subsession = *request->subsession();
      Boolean streamUsingTCP = (request->booleanFlags()&0x1) != 0;
      Boolean streamOutgoing = (request->booleanFlags()&0x2) != 0;
      Boolean forceMulticastOnUnspecified = (request->booleanFlags()&0x4) != 0;

      char const *prefix, *separator, *suffix;
      constructSubsessionURL(subsession, prefix, separator, suffix);

      char const* transportFmt;
      if (strcmp(subsession.protocolName(), "UDP") == 0) {
	suffix = "";
	transportFmt = "Transport: RAW/RAW/UDP%s%s%s=%d-%d\r\n";
      } else {
	transportFmt = "Transport: RTP/AVP%s%s%s=%d-%d\r\n";
      }

      cmdURL = new char[strlen(prefix) + strlen(separator) + strlen(suffix) + 1];
      cmdURLWasAllocated = True;
      sprintf(cmdURL, "%s%s%s", prefix, separator, suffix);

      // Construct a "Transport:" header.
      char const* transportTypeStr;
      char const* modeStr = streamOutgoing ? ";mode=receive" : "";
          // Note: I think the above is nonstandard, but DSS wants it this way
      char const* portTypeStr;
      portNumBits rtpNumber, rtcpNumber;
      if (streamUsingTCP) { // streaming over the RTSP connection
	transportTypeStr = "/TCP;unicast";
	portTypeStr = ";interleaved";
	rtpNumber = fTCPStreamIdCount++;
	rtcpNumber = fTCPStreamIdCount++;
      } else { // normal RTP streaming
	unsigned connectionAddress = subsession.connectionEndpointAddress();
        Boolean requestMulticastStreaming
	  = IsMulticastAddress(connectionAddress) || (connectionAddress == 0 && forceMulticastOnUnspecified);
	transportTypeStr = requestMulticastStreaming ? ";multicast" : ";unicast";
	portTypeStr = ";client_port";
	rtpNumber = subsession.clientPortNum();
	if (rtpNumber == 0) {
	  envir().setResultMsg("Client port number unknown\n");
	  delete[] cmdURL;
	  break;
	}
	rtcpNumber = rtpNumber + 1;
      }
      unsigned transportSize = strlen(transportFmt)
	+ strlen(transportTypeStr) + strlen(modeStr) + strlen(portTypeStr) + 2*5 /* max port len */;
      char* transportStr = new char[transportSize];
      sprintf(transportStr, transportFmt,
	      transportTypeStr, modeStr, portTypeStr, rtpNumber, rtcpNumber);

      // When sending more than one "SETUP" request, include a "Session:" header in the 2nd and later commands:
      char* sessionStr = createSessionString(fLastSessionId);

      // The "Transport:" and "Session:" (if present) headers make up the 'extra headers':
      extraHeaders = new char[transportSize + strlen(sessionStr)];
      extraHeadersWereAllocated = True;
      sprintf(extraHeaders, "%s%s", transportStr, sessionStr);
      delete[] transportStr; delete[] sessionStr;
    } else if (strcmp(request->commandName(), "GET") == 0 || strcmp(request->commandName(), "POST") == 0) {
      // We will be sending a HTTP (not a RTSP) request.
      // Begin by re-parsing our RTSP URL, just to get the stream name, which we'll use as our 'cmdURL' in the subsequent request:
      char* username;
      char* password;
      NetAddress destAddress;
      portNumBits urlPortNum;
      if (!parseRTSPURL(envir(), fBaseURL, username, password, destAddress, urlPortNum, (char const**)&cmdURL)) break;
      if (cmdURL[0] == '\0') cmdURL = (char*)"/";
      delete[] username;
      delete[] password;

      protocolStr = "HTTP/1.0";

      if (strcmp(request->commandName(), "GET") == 0) {
	// Create a 'session cookie' string, using MD5:
	struct {
	  struct timeval timestamp;
	  unsigned counter;
	} seedData;
	gettimeofday(&seedData.timestamp, NULL);
	seedData.counter = ++fSessionCookieCounter;
	our_MD5Data((unsigned char*)(&seedData), sizeof seedData, fSessionCookie);
	// DSS seems to require that the 'session cookie' string be 22 bytes long:
	fSessionCookie[23] = '\0';

	char const* const extraHeadersFmt =
	  "x-sessioncookie: %s\r\n"
	  "Accept: application/x-rtsp-tunnelled\r\n"
	  "Pragma: no-cache\r\n"
	  "Cache-Control: no-cache\r\n";
	unsigned extraHeadersSize = strlen(extraHeadersFmt)
	  + strlen(fSessionCookie);
	extraHeaders = new char[extraHeadersSize];
	extraHeadersWereAllocated = True;
	sprintf(extraHeaders, extraHeadersFmt,
	fSessionCookie);
      } else { // "POST"
	char const* const extraHeadersFmt =
	  "x-sessioncookie: %s\r\n"
	  "Content-Type: application/x-rtsp-tunnelled\r\n"
	  "Pragma: no-cache\r\n"
	  "Cache-Control: no-cache\r\n"
	  "Content-Length: 32767\r\n"
	  "Expires: Sun, 9 Jan 1972 00:00:00 GMT\r\n";
	unsigned extraHeadersSize = strlen(extraHeadersFmt)
	  + strlen(fSessionCookie);
	extraHeaders = new char[extraHeadersSize];
	extraHeadersWereAllocated = True;
	sprintf(extraHeaders, extraHeadersFmt,
		fSessionCookie);
      }
    } else { // "PLAY", "PAUSE", "TEARDOWN", "RECORD", "SET_PARAMETER", "GET_PARAMETER"
      // First, make sure that we have a RTSP session in progress
      if (fLastSessionId == NULL) {
	envir().setResultMsg("No RTSP session is currently in progress\n");
	break;
      }

      char const* sessionId;
      float originalScale;
      if (request->session() != NULL) {
	// Session-level operation
	cmdURL = (char*)sessionURL(*request->session());

	sessionId = fLastSessionId;
	originalScale = request->session()->scale();
      } else {
	// Media-level operation
	char const *prefix, *separator, *suffix;
	constructSubsessionURL(*request->subsession(), prefix, separator, suffix);
	cmdURL = new char[strlen(prefix) + strlen(separator) + strlen(suffix) + 1];
	cmdURLWasAllocated = True;
	sprintf(cmdURL, "%s%s%s", prefix, separator, suffix);

	sessionId = request->subsession()->sessionId;
	originalScale = request->subsession()->scale();
      }

      if (strcmp(request->commandName(), "PLAY") == 0) {
	// Create "Session:", "Scale:", and "Range:" headers; these make up the 'extra headers':
	char* sessionStr = createSessionString(sessionId);
	char* scaleStr = createScaleString(request->scale(), originalScale);
	char* rangeStr = createRangeString(request->start(), request->end());
	extraHeaders = new char[strlen(sessionStr) + strlen(scaleStr) + strlen(rangeStr) + 1];
	extraHeadersWereAllocated = True;
	sprintf(extraHeaders, "%s%s%s", sessionStr, scaleStr, rangeStr);
	delete[] sessionStr; delete[] scaleStr; delete[] rangeStr;
      } else {
	// Create a "Session:" header; this makes up our 'extra headers':
	extraHeaders = createSessionString(sessionId);
	extraHeadersWereAllocated = True;
      }
    }

    char* authenticatorStr = createAuthenticatorString(request->commandName(), fBaseURL);

    char const* const cmdFmt =
      "%s %s %s\r\n"
      "CSeq: %d\r\n"
      "%s"
      "%s"
      "%s"
      "%s"
      "\r\n"
      "%s";
    unsigned cmdSize = strlen(cmdFmt)
      + strlen(request->commandName()) + strlen(cmdURL) + strlen(protocolStr)
      + 20 /* max int len */
      + strlen(authenticatorStr)
      + fUserAgentHeaderStrLen
      + strlen(extraHeaders)
      + strlen(contentLengthHeader)
      + contentStrLen;
    cmd = new char[cmdSize];
    sprintf(cmd, cmdFmt,
	    request->commandName(), cmdURL, protocolStr,
	    request->cseq(),
	    authenticatorStr,
	    fUserAgentHeaderStr,
            extraHeaders,
	    contentLengthHeader,
	    contentStr);
    delete[] authenticatorStr;
    if (cmdURLWasAllocated) delete[] cmdURL;
    if (extraHeadersWereAllocated) delete[] extraHeaders;
    if (contentLengthHeaderWasAllocated) delete[] contentLengthHeader;

    if (fVerbosityLevel >= 1) envir() << "Sending request: " << cmd << "\n";

    if (fTunnelOverHTTPPortNum != 0 && strcmp(request->commandName(), "GET") != 0 && strcmp(request->commandName(), "POST") != 0) {
      // When we're tunneling RTSP-over-HTTP, we Base-64-encode the request before we send it.
      // (However, we don't do this for the HTTP "GET" and "POST" commands that we use to set up the tunnel.)
      char* origCmd = cmd;
      cmd = base64Encode(origCmd, strlen(cmd));
      if (fVerbosityLevel >= 1) envir() << "\tThe request was base-64 encoded to: " << cmd << "\n\n";
      delete[] origCmd;
    }

    if (send(fOutputSocketNum, cmd, strlen(cmd), 0) < 0) {
      char const* errFmt = "%s send() failed: ";
      unsigned const errLength = strlen(errFmt) + strlen(request->commandName());
      char* err = new char[errLength];
      sprintf(err, errFmt, request->commandName());
      envir().setResultErrMsg(err);
      delete[] err;
      break;
    }

    // The command send succeeded, so enqueue the request record, so that its response (when it comes) can be handled:
    fRequestsAwaitingResponse.enqueue(request);

    delete[] cmd;
    return request->cseq();
  } while (0);

  // An error occurred, so call the response handler immediately (indicating the error):
  delete[] cmd;
  handleRequestError(request);
  delete request;
  return 0;
}

void RTSPClient::handleRequestError(RequestRecord* request) {
  int resultCode = -envir().getErrno();
  if (resultCode == 0) {
    // Choose some generic error code instead:
#if defined(__WIN32__) || defined(_WIN32) || defined(_QNX4)
    resultCode = -WSAENOTCONN;
#else
    resultCode = -ENOTCONN;
#endif
  }
  if (request->handler() != NULL) (*request->handler())(this, resultCode, strDup(envir().getResultMsg()));
}

Boolean RTSPClient
::parseResponseCode(char const* line, unsigned& responseCode, char const*& responseString) {
  if (sscanf(line, "RTSP/%*s%u", &responseCode) != 1 &&
      sscanf(line, "HTTP/%*s%u", &responseCode) != 1) return False;
  // Note: We check for HTTP responses as well as RTSP responses, both in order to setup RTSP-over-HTTP tunneling,
  // and so that we get back a meaningful error if the client tried to mistakenly send a RTSP command to a HTTP-only server.

  // Use everything after the RTSP/* (or HTTP/*) as the response string:
  responseString = line;
  while (responseString[0] != '\0' && responseString[0] != ' '  && responseString[0] != '\t') ++responseString;
  while (responseString[0] != '\0' && (responseString[0] == ' '  || responseString[0] == '\t')) ++responseString; // skip whitespace

  return True;
}

void RTSPClient::handleIncomingRequest() {
  // Parse the request string into command name and 'CSeq', then 'handle' the command (by responding that we don't support it):
  char cmdName[RTSP_PARAM_STRING_MAX];
  char urlPreSuffix[RTSP_PARAM_STRING_MAX];
  char urlSuffix[RTSP_PARAM_STRING_MAX];
  char cseq[RTSP_PARAM_STRING_MAX];
  unsigned contentLength;
  if (!parseRTSPRequestString(fResponseBuffer, fResponseBytesAlreadySeen,
			      cmdName, sizeof cmdName,
			      urlPreSuffix, sizeof urlPreSuffix,
			      urlSuffix, sizeof urlSuffix,
			      cseq, sizeof cseq,
			      contentLength)) {
    return;
  } else {
    if (fVerbosityLevel >= 1) {
      envir() << "Received incoming RTSP request: " << fResponseBuffer << "\n";
    }
    char tmpBuf[2*RTSP_PARAM_STRING_MAX];
    snprintf((char*)tmpBuf, sizeof tmpBuf,
             "RTSP/1.0 405 Method Not Allowed\r\nCSeq: %s\r\n\r\n", cseq);
    send(fOutputSocketNum, tmpBuf, strlen(tmpBuf), 0);
  }
}

Boolean RTSPClient::checkForHeader(char const* line, char const* headerName, unsigned headerNameLength, char const*& headerParams) {
  if (_strncasecmp(line, headerName, headerNameLength) != 0) return False;

  // The line begins with the desired header name.  Trim off any whitespace, and return the header parameters:
  unsigned paramIndex = headerNameLength;
  while (line[paramIndex] != '\0' && (line[paramIndex] == ' ' || line[paramIndex] == '\t')) ++paramIndex;
  if (&line[paramIndex] == '\0') return False; // the header is assumed to be bad if it has no parameters

  headerParams = &line[paramIndex];
  return True;
}

Boolean RTSPClient::parseTransportParams(char const* paramsStr,
					 char*& serverAddressStr, portNumBits& serverPortNum,
					 unsigned char& rtpChannelId, unsigned char& rtcpChannelId) {
  // Initialize the return parameters to 'not found' values:
  serverAddressStr = NULL;
  serverPortNum = 0;
  rtpChannelId = rtcpChannelId = 0xFF;

  char* foundServerAddressStr = NULL;
  Boolean foundServerPortNum = False;
  portNumBits clientPortNum = 0;
  Boolean foundClientPortNum = False;
  Boolean foundChannelIds = False;
  unsigned rtpCid, rtcpCid;
  Boolean isMulticast = True; // by default
  char* foundDestinationStr = NULL;
  portNumBits multicastPortNumRTP, multicastPortNumRTCP;
  Boolean foundMulticastPortNum = False;

  // Run through each of the parameters, looking for ones that we handle:
  char const* fields = paramsStr;
  char* field = strDupSize(fields);
  while (sscanf(fields, "%[^;]", field) == 1) {
    if (sscanf(field, "server_port=%hu", &serverPortNum) == 1) {
      foundServerPortNum = True;
    } else if (sscanf(field, "client_port=%hu", &clientPortNum) == 1) {
      foundClientPortNum = True;
    } else if (_strncasecmp(field, "source=", 7) == 0) {
      delete[] foundServerAddressStr;
      foundServerAddressStr = strDup(field+7);
    } else if (sscanf(field, "interleaved=%u-%u", &rtpCid, &rtcpCid) == 2) {
      rtpChannelId = (unsigned char)rtpCid;
      rtcpChannelId = (unsigned char)rtcpCid;
      foundChannelIds = True;
    } else if (strcmp(field, "unicast") == 0) {
      isMulticast = False;
    } else if (_strncasecmp(field, "destination=", 12) == 0) {
      delete[] foundDestinationStr;
      foundDestinationStr = strDup(field+12);
    } else if (sscanf(field, "port=%hu-%hu", &multicastPortNumRTP, &multicastPortNumRTCP) == 2 ||
	       sscanf(field, "port=%hu", &multicastPortNumRTP) == 1) {
      foundMulticastPortNum = True;
    }

    fields += strlen(field);
    while (fields[0] == ';') ++fields; // skip over all leading ';' chars
    if (fields[0] == '\0') break;
  }
  delete[] field;

  // If we're multicast, and have a "destination=" (multicast) address, then use this
  // as the 'server' address (because some weird servers don't specify the multicast
  // address earlier, in the "DESCRIBE" response's SDP:
  if (isMulticast && foundDestinationStr != NULL && foundMulticastPortNum) {
    delete[] foundServerAddressStr;
    serverAddressStr = foundDestinationStr;
    serverPortNum = multicastPortNumRTP;
    return True;
  }
  delete[] foundDestinationStr;

  // We have a valid "Transport:" header if any of the following are true:
  //   - We saw a "interleaved=" field, indicating RTP/RTCP-over-TCP streaming, or
  //   - We saw a "server_port=" field, or
  //   - We saw a "client_port=" field.
  //     If we didn't also see a "server_port=" field, then the server port is assumed to be the same as the client port.
  if (foundChannelIds || foundServerPortNum || foundClientPortNum) {
    if (foundClientPortNum && !foundServerPortNum) {
      serverPortNum = clientPortNum;
    }
    serverAddressStr = foundServerAddressStr;
    return True;
  }

  delete[] foundServerAddressStr;
  return False;
}

Boolean RTSPClient::parseScaleParam(char const* paramStr, float& scale) {
  Locale l("C", Numeric);
  return sscanf(paramStr, "%f", &scale) == 1;
}

Boolean RTSPClient::parseRTPInfoParams(char const*& paramsStr, u_int16_t& seqNum, u_int32_t& timestamp) {
  while (paramsStr[0] == ',') ++paramsStr;

  // "paramsStr" now consists of a ';'-separated list of parameters, ending with ',' or '\0'.
  char* field = strDupSize(paramsStr);

  while (sscanf(paramsStr, "%[^;,]", field) == 1) {
    if (sscanf(field, "seq=%hu", &seqNum) == 1 ||
	sscanf(field, "rtptime=%u", &timestamp) == 1) {
    }

    paramsStr += strlen(field);
    if (paramsStr[0] == '\0' || paramsStr[0] == ',') break;
    // ASSERT: paramsStr[0] == ';'
    ++paramsStr; // skip over the ';'
  }

  delete[] field;
  return True;
}

Boolean RTSPClient::handleSETUPResponse(MediaSubsession& subsession, char const* sessionParamsStr, char const* transportParamsStr,
                                        Boolean streamUsingTCP) {
  char* sessionId = new char[responseBufferSize]; // ensures we have enough space
  Boolean success = False;
  do {
    // Check for a session id:
    if (sessionParamsStr == NULL || sscanf(sessionParamsStr, "%[^;]", sessionId) != 1) {
      envir().setResultMsg("Missing or bad \"Session:\" header");
      break;
    }
    subsession.sessionId = strDup(sessionId);
    delete[] fLastSessionId; fLastSessionId = strDup(sessionId);

    // Also look for an optional "; timeout = " parameter following this:
    char const* afterSessionId = sessionParamsStr + strlen(sessionId);
    int timeoutVal;
    if (sscanf(afterSessionId, "; timeout = %d", &timeoutVal) == 1) {
      fSessionTimeoutParameter = timeoutVal;
    }

    // Parse the "Transport:" header parameters:
    char* serverAddressStr;
    portNumBits serverPortNum;
    unsigned char rtpChannelId, rtcpChannelId;
    if (!parseTransportParams(transportParamsStr, serverAddressStr, serverPortNum, rtpChannelId, rtcpChannelId)) {
      envir().setResultMsg("Missing or bad \"Transport:\" header");
      break;
    }
    delete[] subsession.connectionEndpointName();
    subsession.connectionEndpointName() = serverAddressStr;
    subsession.serverPortNum = serverPortNum;
    subsession.rtpChannelId = rtpChannelId;
    subsession.rtcpChannelId = rtcpChannelId;

    if (streamUsingTCP) {
      // Tell the subsession to receive RTP (and send/receive RTCP) over the RTSP stream:
      if (subsession.rtpSource() != NULL) {
	subsession.rtpSource()->setStreamSocket(fInputSocketNum, subsession.rtpChannelId);
	subsession.rtpSource()->setServerRequestAlternativeByteHandler(fInputSocketNum, handleAlternativeRequestByte, this);
      }
      if (subsession.rtcpInstance() != NULL) subsession.rtcpInstance()->setStreamSocket(fInputSocketNum, subsession.rtcpChannelId);
    } else {
      // Normal case.
      // Set the RTP and RTCP sockets' destination address and port from the information in the SETUP response (if present):
      netAddressBits destAddress = subsession.connectionEndpointAddress();
      if (destAddress == 0) destAddress = fServerAddress;
      subsession.setDestinations(destAddress);
    }

    success = True;
  } while (0);

  delete[] sessionId;
  return success;
}

Boolean RTSPClient::handlePLAYResponse(MediaSession& session, MediaSubsession& subsession,
                                       char const* scaleParamsStr, char const* rangeParamsStr, char const* rtpInfoParamsStr) {
  Boolean scaleOK = False, rangeOK = False;
  do {
    if (&session != NULL) {
      // The command was on the whole session
      if (scaleParamsStr != NULL && !parseScaleParam(scaleParamsStr, session.scale())) break;
      scaleOK = True;
      if (rangeParamsStr != NULL && !parseRangeParam(rangeParamsStr, session.playStartTime(), session.playEndTime())) break;
      rangeOK = True;

      u_int16_t seqNum; u_int32_t timestamp;
      if (rtpInfoParamsStr != NULL) {
	if (!parseRTPInfoParams(rtpInfoParamsStr, seqNum, timestamp)) break;
	// This is data for our first subsession.  Fill it in, and do the same for our other subsessions:
	MediaSubsessionIterator iter(session);
	MediaSubsession* subsession;
	while ((subsession = iter.next()) != NULL) {
	  subsession->rtpInfo.seqNum = seqNum;
	  subsession->rtpInfo.timestamp = timestamp;
	  subsession->rtpInfo.infoIsNew = True;

	  if (!parseRTPInfoParams(rtpInfoParamsStr, seqNum, timestamp)) break;
	}
      }
    } else {
      // The command was on a subsession
      if (scaleParamsStr != NULL && !parseScaleParam(scaleParamsStr, subsession.scale())) break;
      scaleOK = True;
      if (rangeParamsStr != NULL && !parseRangeParam(rangeParamsStr, subsession._playStartTime(), subsession._playEndTime())) break;
      rangeOK = True;

      u_int16_t seqNum; u_int32_t timestamp;
      if (rtpInfoParamsStr != NULL) {
	if (!parseRTPInfoParams(rtpInfoParamsStr, seqNum, timestamp)) break;
	subsession.rtpInfo.seqNum = seqNum;
	subsession.rtpInfo.timestamp = timestamp;
	subsession.rtpInfo.infoIsNew = True;
      }
    }

    return True;
  } while (0);

  // An error occurred:
  if (!scaleOK) {
    envir().setResultMsg("Bad \"Scale:\" header");
  } else if (!rangeOK) {
    envir().setResultMsg("Bad \"Range:\" header");
  } else {
    envir().setResultMsg("Bad \"RTP-Info:\" header");
  }
  return False;
}

Boolean RTSPClient::handleTEARDOWNResponse(MediaSession& session, MediaSubsession& subsession) {
  if (&session != NULL) {
    // The command was on the whole session
    // Run through each subsession, deleting its "sessionId":
    MediaSubsessionIterator iter(session);
    MediaSubsession* subsession;
    while ((subsession = iter.next()) != NULL) {
      delete[] (char*)subsession->sessionId;
      subsession->sessionId = NULL;
    }
  } else {
    // The command was on a subsession
    delete[] (char*)subsession.sessionId;
    subsession.sessionId = NULL;
  }
  return True;
}

Boolean RTSPClient::handleGET_PARAMETERResponse(char const* parameterName, char*& resultValueString) {
  do {
    // If "parameterName" is non-empty, it should be (possibly followed by ':' and whitespace) at the start of the result string:
    if (parameterName != NULL && parameterName[0] != '\0') {
      if (parameterName[1] == '\0') break; // sanity check; there should have been \r\n at the end of "parameterName"

      unsigned parameterNameLen = strlen(parameterName);
      // ASSERT: parameterNameLen >= 2;
      parameterNameLen -= 2; // because of the trailing \r\n
      if (_strncasecmp(resultValueString, parameterName, parameterNameLen) != 0) {
	// The parameter name wasn't in the output, so just return an empty string:
	resultValueString[0] = '\0';
	return True;
      }
      resultValueString += parameterNameLen;
      if (resultValueString[0] == ':') ++resultValueString;
      while (resultValueString[0] == ' ' || resultValueString[0] == '\t') ++resultValueString;
    }

    // The rest of "resultValueStr" should be our desired result, but first trim off any \r and/or \n characters at the end:
    unsigned resultLen = strlen(resultValueString);
    while (resultLen > 0 && (resultValueString[resultLen-1] == '\r' || resultValueString[resultLen-1] == '\n')) --resultLen;
    resultValueString[resultLen] = '\0';

    return True;
  } while (0);

  // An error occurred:
  envir().setResultMsg("Bad \"GET_PARAMETER\" response");
  return False;
}

Boolean RTSPClient::handleAuthenticationFailure(char const* paramsStr) {
  if (paramsStr == NULL) return False; // There was no "WWW-Authenticate:" header; we can't proceed.

  // Fill in "fCurrentAuthenticator" with the information from the "WWW-Authenticate:" header:
  Boolean alreadyHadRealm = fCurrentAuthenticator.realm() != NULL;
  char* realm = strDupSize(paramsStr);
  char* nonce = strDupSize(paramsStr);
  Boolean success = True;
  if (sscanf(paramsStr, "Digest realm=\"%[^\"]\", nonce=\"%[^\"]\"", realm, nonce) == 2) {
    fCurrentAuthenticator.setRealmAndNonce(realm, nonce);
  } else if (sscanf(paramsStr, "Basic realm=\"%[^\"]\"", realm) == 1) {
    fCurrentAuthenticator.setRealmAndNonce(realm, NULL); // Basic authentication
  } else {
    success = False; // bad "WWW-Authenticate:" header
  }
  delete[] realm; delete[] nonce;

  if (alreadyHadRealm || fCurrentAuthenticator.username() == NULL || fCurrentAuthenticator.password() == NULL) {
    // We already had a 'realm', or don't have a username and/or password,
    // so the new "WWW-Authenticate:" header information won't help us.  We remain unauthenticated.
    success = False;
  }

  return success;
}

Boolean RTSPClient::resendCommand(RequestRecord* request) {
  if (fVerbosityLevel >= 1) envir() << "Resending...\n";
  if (request != NULL && strcmp(request->commandName(), "GET") != 0) request->cseq() = ++fCSeq;
  return sendRequest(request) != 0;
}

char const* RTSPClient::sessionURL(MediaSession const& session) const {
  char const* url = session.controlPath();
  if (url == NULL || strcmp(url, "*") == 0) url = fBaseURL;

  return url;
}

void RTSPClient::handleAlternativeRequestByte(void* rtspClient, u_int8_t requestByte) {
  ((RTSPClient*)rtspClient)->handleAlternativeRequestByte1(requestByte);
}

void RTSPClient::handleAlternativeRequestByte1(u_int8_t requestByte) {
  fResponseBuffer[fResponseBytesAlreadySeen] = requestByte;
  handleResponseBytes(1);
}

static Boolean isAbsoluteURL(char const* url) {
  // Assumption: "url" is absolute if it contains a ':', before any
  // occurrence of '/'
  while (*url != '\0' && *url != '/') {
    if (*url == ':') return True;
    ++url;
  }

  return False;
}

void RTSPClient::constructSubsessionURL(MediaSubsession const& subsession,
					char const*& prefix,
					char const*& separator,
					char const*& suffix) {
  // Figure out what the URL describing "subsession" will look like.
  // The URL is returned in three parts: prefix; separator; suffix
  //##### NOTE: This code doesn't really do the right thing if "sessionURL()"
  // doesn't end with a "/", and "subsession.controlPath()" is relative.
  // The right thing would have been to truncate "sessionURL()" back to the
  // rightmost "/", and then add "subsession.controlPath()".
  // In practice, though, each "DESCRIBE" response typically contains
  // a "Content-Base:" header that consists of "sessionURL()" followed by
  // a "/", in which case this code ends up giving the correct result.
  // However, we should really fix this code to do the right thing, and
  // also check for and use the "Content-Base:" header appropriately. #####
  prefix = sessionURL(subsession.parentSession());
  if (prefix == NULL) prefix = "";

  suffix = subsession.controlPath();
  if (suffix == NULL) suffix = "";

  if (isAbsoluteURL(suffix)) {
    prefix = separator = "";
  } else {
    unsigned prefixLen = strlen(prefix);
    separator = (prefixLen == 0 || prefix[prefixLen-1] == '/' || suffix[0] == '/') ? "" : "/";
  }
}

Boolean RTSPClient::setupHTTPTunneling1() {
  // Set up RTSP-over-HTTP tunneling, as described in
  //     http://developer.apple.com/quicktime/icefloe/dispatch028.html and http://images.apple.com/br/quicktime/pdf/QTSS_Modules.pdf
  if (fVerbosityLevel >= 1) {
    envir() << "Requesting RTSP-over-HTTP tunneling (on port " << fTunnelOverHTTPPortNum << ")\n\n";
  }

  // Begin by sending a HTTP "GET", to set up the server->client link.  Continue when we handle the response:
  return sendRequest(new RequestRecord(1, "GET", responseHandlerForHTTP_GET)) != 0;
}

void RTSPClient::responseHandlerForHTTP_GET(RTSPClient* rtspClient, int responseCode, char* responseString) {
  if (rtspClient != NULL) rtspClient->responseHandlerForHTTP_GET1(responseCode, responseString);
}

void RTSPClient::responseHandlerForHTTP_GET1(int responseCode, char* responseString) {
  RequestRecord* request;
  do {
    if (responseCode != 0) break; // The HTTP "GET" failed.

    // Having successfully set up (using the HTTP "GET" command) the server->client link, set up a second TCP connection
    // (to the same server & port as before) for the client->server link.  All future output will be to this new socket.
    fOutputSocketNum = setupStreamSocket(envir(), 0);
    if (fOutputSocketNum < 0) break;

    fHTTPTunnelingConnectionIsPending = True;
    int connectResult = connectToServer(fOutputSocketNum, fTunnelOverHTTPPortNum);
    if (connectResult < 0) break; // an error occurred
    else if (connectResult == 0) {
      // A connection is pending.  Continue setting up RTSP-over-HTTP when the connection completes.
      // First, move the pending requests to the 'awaiting connection' queue:
      while ((request = fRequestsAwaitingHTTPTunneling.dequeue()) != NULL) {
	fRequestsAwaitingConnection.enqueue(request);
      }
      return;
    }

    // The connection succeeded.  Continue setting up RTSP-over-HTTP:
    if (!setupHTTPTunneling2()) break;

    // RTSP-over-HTTP tunneling succeeded.  Resume the pending request(s):
    while ((request = fRequestsAwaitingHTTPTunneling.dequeue()) != NULL) {
      sendRequest(request);
    }
    return;
  } while (0);

  // An error occurred.  Dequeue the pending request(s), and tell them about the error:
  fHTTPTunnelingConnectionIsPending = False;
  while ((request = fRequestsAwaitingHTTPTunneling.dequeue()) != NULL) {
    handleRequestError(request);
    delete request;
  }
  resetTCPSockets();
}

Boolean RTSPClient::setupHTTPTunneling2() {
  fHTTPTunnelingConnectionIsPending = False;

  // Send a HTTP "POST", to set up the client->server link.  (Note that we won't see a reply to the "POST".)
  return sendRequest(new RequestRecord(1, "POST", NULL)) != 0;
}

void RTSPClient::connectionHandler(void* instance, int /*mask*/) {
  RTSPClient* client = (RTSPClient*)instance;
  client->connectionHandler1();
}

void RTSPClient::connectionHandler1() {
  // Restore normal handling on our sockets:
  envir().taskScheduler().disableBackgroundHandling(fOutputSocketNum);
  envir().taskScheduler().setBackgroundHandling(fInputSocketNum, SOCKET_READABLE,
						(TaskScheduler::BackgroundHandlerProc*)&incomingDataHandler, this);

  // Move all requests awaiting connection into a new, temporary queue, to clear "fRequestsAwaitingConnection"
  // (so that "sendRequest()" doesn't get confused by "fRequestsAwaitingConnection" being nonempty, and enqueue them all over again).
  RequestQueue tmpRequestQueue;
  RequestRecord* request;
  while ((request = fRequestsAwaitingConnection.dequeue()) != NULL) {
    tmpRequestQueue.enqueue(request);
  }

  // Find out whether the connection succeeded or failed:
  do {
    int err = 0;
    SOCKLEN_T len = sizeof err;
    if (getsockopt(fInputSocketNum, SOL_SOCKET, SO_ERROR, (char*)&err, &len) < 0 || err != 0) {
      envir().setResultErrMsg("Connection to server failed: ", err);
      if (fVerbosityLevel >= 1) envir() << "..." << envir().getResultMsg() << "\n";
      break;
    }

    // The connection succeeded.  If the connection came about from an attempt to set up RTSP-over-HTTP, finish this now:
    if (fVerbosityLevel >= 1) envir() << "...remote connection opened\n";
    if (fHTTPTunnelingConnectionIsPending && !setupHTTPTunneling2()) break;

    // Resume sending all pending requests:
    while ((request = tmpRequestQueue.dequeue()) != NULL) {
      sendRequest(request);
    }
    return;
  } while (0);

  // An error occurred.  Tell all pending requests about the error:
  while ((request = tmpRequestQueue.dequeue()) != NULL) {
    handleRequestError(request);
    delete request;
  }
  resetTCPSockets();
}

void RTSPClient::incomingDataHandler(void* instance, int /*mask*/) {
  RTSPClient* client = (RTSPClient*)instance;
  client->incomingDataHandler1();
}

void RTSPClient::incomingDataHandler1() {
  struct sockaddr_in dummy; // 'from' address - not used

  int bytesRead = readSocket(envir(), fInputSocketNum, (unsigned char*)&fResponseBuffer[fResponseBytesAlreadySeen], fResponseBufferBytesLeft, dummy);
  handleResponseBytes(bytesRead);
}

static char* getLine(char* startOfLine) {
  // returns the start of the next line, or NULL if none.  Note that this modifies the input string to add '\0' characters.
  for (char* ptr = startOfLine; *ptr != '\0'; ++ptr) {
    // Check for the end of line: \r\n (but also accept \r or \n by itself):
    if (*ptr == '\r' || *ptr == '\n') {
      // We found the end of the line
      if (*ptr == '\r') {
	*ptr++ = '\0';
	if (*ptr == '\n') ++ptr;
      } else {
        *ptr++ = '\0';
      }
      return ptr;
    }
  }

  return NULL;
}

void RTSPClient::handleResponseBytes(int newBytesRead) {
  do {
    if (newBytesRead > 0 && (unsigned)newBytesRead < fResponseBufferBytesLeft) break; // data was read OK; process it below

    if ((unsigned)newBytesRead >= fResponseBufferBytesLeft) {
      // We filled up our response buffer.  Treat this as an error (for the first response handler):
      envir().setResultMsg("RTSP response was truncated. Increase \"RTSPClient::responseBufferSize\"");
    }

    // An error occurred while reading our TCP socket.  Call all pending response handlers, indicating this error:
    RequestRecord* request;
    while ((request = fRequestsAwaitingResponse.dequeue()) != NULL) {
      handleRequestError(request);
      delete request;

      if (newBytesRead > 0) break; // The "RTSP response was truncated" error is applied to the first response handler only
    }

    if (newBytesRead <= 0) resetTCPSockets();
    resetResponseBuffer();
    return;
  } while (0);

  fResponseBufferBytesLeft -= newBytesRead;
  fResponseBytesAlreadySeen += newBytesRead;
  fResponseBuffer[fResponseBytesAlreadySeen] = '\0';
  if (fVerbosityLevel >= 1 && newBytesRead > 1) envir() << "Received " << newBytesRead << " new bytes of response data.\n";

  // Data was read OK.  Look through the data that we've read so far, to see if it contains <CR><LF><CR><LF>.
  // (If not, wait for more data to arrive.)
  Boolean endOfHeaders = False;
  if (fResponseBytesAlreadySeen > 3) {
    char const* const ptrEnd = &fResponseBuffer[fResponseBytesAlreadySeen-3];
    char const* ptr = fResponseBuffer;
    while (ptr < ptrEnd) {
      if (*ptr++ == '\r' && *ptr++ == '\n' && *ptr++ == '\r' && *ptr++ == '\n') {
	// This is it
        endOfHeaders = True;
        break;
      }
    }
  }

  if (!endOfHeaders) return; // subsequent reads will be needed to get the complete response

  // Now that we have the complete response headers (ending with <CR><LF><CR><LF>), parse them to get the response code, CSeq,
  // and various other header parameters.  To do this, we first make a copy of the received header data, because we'll be modifying
  // it by adding '\0' bytes.
  char* headerDataCopy;
  unsigned responseCode = 200;
  char const* responseStr = NULL;
  RequestRecord* foundRequest = NULL;
  char const* sessionParamsStr = NULL;
  char const* transportParamsStr = NULL;
  char const* scaleParamsStr = NULL;
  char const* rangeParamsStr = NULL;
  char const* rtpInfoParamsStr = NULL;
  char const* wwwAuthenticateParamsStr = NULL;
  char const* publicParamsStr = NULL;
  char* bodyStart = NULL;
  unsigned numBodyBytes = 0;
  Boolean responseSuccess = False; // by default
  do {
    headerDataCopy = new char[responseBufferSize];
    strncpy(headerDataCopy, fResponseBuffer, fResponseBytesAlreadySeen);
    headerDataCopy[fResponseBytesAlreadySeen] = '\0';

    char* lineStart = headerDataCopy;
    char* nextLineStart = getLine(lineStart);
    if (!parseResponseCode(lineStart, responseCode, responseStr)) {
      // This does not appear to be a RTSP response; perhaps it's a RTSP request instead?
      handleIncomingRequest();
      break; // we're done with this data
    }

    // Scan through the headers, handling the ones that we're interested in:
    Boolean reachedEndOfHeaders;
    unsigned cseq = 0;
    unsigned contentLength = 0;

    while (1) {
      reachedEndOfHeaders = True; // by default; may get changed below
      lineStart = nextLineStart;
      if (lineStart == NULL) break;

      nextLineStart = getLine(lineStart);
      if (lineStart[0] == '\0') break; // this is a blank line
      reachedEndOfHeaders = False;

      char const* headerParamsStr;
      if (checkForHeader(lineStart, "CSeq:", 5, headerParamsStr)) {
        if (sscanf(headerParamsStr, "%u", &cseq) != 1 || cseq <= 0) {
	  envir().setResultMsg("Bad \"CSeq:\" header: \"", lineStart, "\"");
	  break;
	}
        // Find the handler function for "cseq":
	RequestRecord* request;
	while ((request = fRequestsAwaitingResponse.dequeue()) != NULL) {
          if (request->cseq() < cseq) { // assumes that the CSeq counter will never wrap around
            // We never received (and will never receive) a response for this handler, so delete it:
            delete request;
          } else if (request->cseq() == cseq) {
            // This is the handler that we want. Remove its record, but remember it, so that we can later call its handler:
            foundRequest = request;
            break;
          } else { // request->cseq() > cseq
            // No handler was registered for this response, so ignore it.
            break;
          }
        }
      } else if (checkForHeader(lineStart, "Content-Length:", 15, headerParamsStr)) {
        if (sscanf(headerParamsStr, "%u", &contentLength) != 1) {
	  envir().setResultMsg("Bad \"Content-Length:\" header: \"", lineStart, "\"");
	  break;
	}
      } else if (checkForHeader(lineStart, "Content-Base:", 13, headerParamsStr)) {
        setBaseURL(headerParamsStr);
      } else if (checkForHeader(lineStart, "Session:", 8, sessionParamsStr)) {
      } else if (checkForHeader(lineStart, "Transport:", 10, transportParamsStr)) {
      } else if (checkForHeader(lineStart, "Scale:", 6, scaleParamsStr)) {
      } else if (checkForHeader(lineStart, "Range:", 6, rangeParamsStr)) {
      } else if (checkForHeader(lineStart, "RTP-Info:", 9, rtpInfoParamsStr)) {
      } else if (checkForHeader(lineStart, "WWW-Authenticate:", 17, headerParamsStr)) {
	// If we've already seen a "WWW-Authenticate:" header, then we replace it with this new one only if
	// the new one specifies "Digest" authentication:
	if (wwwAuthenticateParamsStr == NULL || _strncasecmp(headerParamsStr, "Digest", 6) == 0) {
	  wwwAuthenticateParamsStr = headerParamsStr;
	}
      } else if (checkForHeader(lineStart, "Public:", 7, publicParamsStr)) {
      } else if (checkForHeader(lineStart, "Allow:", 6, publicParamsStr)) {
	// Note: we accept "Allow:" instead of "Public:", so that "OPTIONS" requests made to HTTP servers will work.
      } else if (checkForHeader(lineStart, "Location:", 9, headerParamsStr)) {
        setBaseURL(headerParamsStr);
      }
    }
    if (!reachedEndOfHeaders) break; // an error occurred

    if (foundRequest == NULL) {
      // Hack: The response didn't have a "CSeq:" header; assume it's for our most recent request:
      foundRequest = fRequestsAwaitingResponse.dequeue();
    }

    // If we saw a "Content-Length:" header, then make sure that we have the amount of data that it specified:
    unsigned bodyOffset = nextLineStart - headerDataCopy;
    bodyStart = &fResponseBuffer[bodyOffset];
    numBodyBytes = fResponseBytesAlreadySeen - bodyOffset;
    if (contentLength > numBodyBytes) {
      // We need to read more data.  First, make sure we have enough space for it:
      unsigned numExtraBytesNeeded = contentLength - numBodyBytes;
      unsigned remainingBufferSize = responseBufferSize - fResponseBytesAlreadySeen;
      if (numExtraBytesNeeded > remainingBufferSize) {
        char tmpBuf[200];
	sprintf(tmpBuf, "Response buffer size (%d) is too small for \"Content-Length:\" %d (need a buffer size of >= %d bytes\n",
                responseBufferSize, contentLength, fResponseBytesAlreadySeen + numExtraBytesNeeded);
	envir().setResultMsg(tmpBuf);
        break;
      }

      if (fVerbosityLevel >= 1) {
        envir() << "Have received " << fResponseBytesAlreadySeen << " total bytes of a "
                << (foundRequest != NULL ? foundRequest->commandName() : "(unknown)")
                << " RTSP response; awaiting " << numExtraBytesNeeded << " bytes more.\n";
      }
      delete[] headerDataCopy;
      if (foundRequest != NULL) fRequestsAwaitingResponse.putAtHead(foundRequest); // put back our request record; we need it again
      return; // We need to read more data
    }

    // We now have a complete response (including all bytes specified by the "Content-Length:" header, if any).
    if (fVerbosityLevel >= 1) {
      envir() << "Received a complete "
	      << (foundRequest != NULL ? foundRequest->commandName() : "(unknown)")
	      << " response:\n" << fResponseBuffer << "\n";
    }

    if (foundRequest != NULL) {
      Boolean needToResendCommand = False; // by default...
      if (responseCode == 200) {
	// Do special-case response handling for some commands:
	if (strcmp(foundRequest->commandName(), "SETUP") == 0) {
	  if (!handleSETUPResponse(*foundRequest->subsession(), sessionParamsStr, transportParamsStr, foundRequest->booleanFlags()&0x1)) break;
	} else if (strcmp(foundRequest->commandName(), "PLAY") == 0) {
	  if (!handlePLAYResponse(*foundRequest->session(), *foundRequest->subsession(), scaleParamsStr, rangeParamsStr, rtpInfoParamsStr)) break;
	} else if (strcmp(foundRequest->commandName(), "TEARDOWN") == 0) {
	  if (!handleTEARDOWNResponse(*foundRequest->session(), *foundRequest->subsession())) break;
	} else if (strcmp(foundRequest->commandName(), "GET_PARAMETER") == 0) {
	  if (!handleGET_PARAMETERResponse(foundRequest->contentStr(), bodyStart)) break;
	}
      } else if (responseCode == 401 && handleAuthenticationFailure(wwwAuthenticateParamsStr)) {
	// We need to resend the command, with an "Authorization:" header:
	needToResendCommand = True;

	if (strcmp(foundRequest->commandName(), "GET") == 0) {
	  // Note: If a HTTP "GET" command (for RTSP-over-HTTP tunneling) returns "401 Unauthorized", then we resend it
	  // (with an "Authorization:" header), just as we would for a RTSP command.  However, we do so using a new TCP connection,
	  // because some servers close the original connection after returning the "401 Unauthorized".
	  resetTCPSockets(); // forces the opening of a new connection for the resent command
	}
      } else if (responseCode == 301 || responseCode == 302) { // redirection
	resetTCPSockets(); // because we need to connect somewhere else next
	needToResendCommand = True;
      }

      if (needToResendCommand) {
	resetResponseBuffer();
	if (!resendCommand(foundRequest)) break;
	delete[] headerDataCopy;
	return; // without calling our response handler; the response to the resent command will do that
      }
    }

    responseSuccess = True;
  } while (0);

  // If we have a handler function for this response, call it:
  resetResponseBuffer(); // in preparation for our next response.  Do this now, in case the handler function goes to the event loop.
  if (foundRequest != NULL && foundRequest->handler() != NULL) {
    int resultCode;
    char* resultString;
    if (responseSuccess) {
      if (responseCode == 200) {
        resultCode = 0;
        resultString = numBodyBytes != 0 ? strDup(bodyStart) : strDup(publicParamsStr);
          // Note: The "strDup(bodyStart)" call assumes that the body is encoded without interior '\0' bytes
      } else {
        resultCode = responseCode;
        resultString = strDup(responseStr);
        envir().setResultMsg(responseStr);
      }
      (*foundRequest->handler())(this, resultCode, resultString);
    } else {
      // An error occurred parsing the response, so call the handler, indicating an error:
      handleRequestError(foundRequest);
    }
  }
  delete foundRequest;
  delete[] headerDataCopy;
}


////////// RTSPClient::RequestRecord implementation //////////

RTSPClient::RequestRecord::RequestRecord(unsigned cseq, char const* commandName, responseHandler* handler,
					 MediaSession* session, MediaSubsession* subsession, u_int32_t booleanFlags,
					 double start, double end, float scale, char const* contentStr)
  : fNext(NULL), fCSeq(cseq), fCommandName(commandName), fSession(session), fSubsession(subsession), fBooleanFlags(booleanFlags),
    fStart(start), fEnd(end), fScale(scale), fContentStr(strDup(contentStr)), fHandler(handler) {
}

RTSPClient::RequestRecord::~RequestRecord() {
  // Delete the rest of the list first:
  delete fNext;

  delete[] fContentStr;
}


////////// RTSPClient::RequestQueue implementation //////////

RTSPClient::RequestQueue::RequestQueue()
  : fHead(NULL), fTail(NULL) {
}

RTSPClient::RequestQueue::~RequestQueue() {
  delete fHead;
}

void RTSPClient::RequestQueue::enqueue(RequestRecord* request) {
  if (fTail == NULL) {
    fHead = request;
  } else {
    fTail->next() = request;
  }
  fTail = request;
}

RTSPClient::RequestRecord* RTSPClient::RequestQueue::dequeue() {
  RequestRecord* request = fHead;
  if (fHead == fTail) {
    fHead = NULL;
    fTail = NULL;
  } else {
    fHead = fHead->next();
  }
  if (request != NULL) request->next() = NULL;
  return request;
}

void RTSPClient::RequestQueue::putAtHead(RequestRecord* request) {
  request->next() = fHead;
  fHead = request;
  if (fTail == NULL) {
    fTail = request;
  }
}

RTSPClient::RequestRecord* RTSPClient::RequestQueue::findByCSeq(unsigned cseq) {
  RequestRecord* request;
  for (request = fHead; request != NULL; request = request->next()) {
    if (request->cseq() == cseq) return request;
  }
  return NULL;
}


#ifdef RTSPCLIENT_SYNCHRONOUS_INTERFACE
// Implementation of the old (synchronous) "RTSPClient" interface, using the new (asynchronous) interface:
RTSPClient* RTSPClient::createNew(UsageEnvironment& env,
				  int verbosityLevel,
				  char const* applicationName,
				  portNumBits tunnelOverHTTPPortNum) {
  return new RTSPClient(env, NULL,
			verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

char* RTSPClient::describeURL(char const* url, Authenticator* authenticator,
			      Boolean allowKasennaProtocol, int timeout) {
  // Sorry 'Kasenna', but the party's over.  You've had 6 years to make your servers compliant with the standard RTSP protocol.
  // We're not going to support your non-standard hacked version of the protocol any more.  Starting now, the "allowKasennaProtocol"
  // parameter is a noop (and eventually, when the synchronous interface goes away completely, then so will this parameter).

  // First, check whether "url" contains a username:password to be used.  If so, handle this using "describeWithPassword()" instead:
  char* username; char* password;
  if (authenticator == NULL
      && parseRTSPURLUsernamePassword(url, username, password)) {
    char* result = describeWithPassword(url, username, password, allowKasennaProtocol, timeout);
    delete[] username; delete[] password; // they were dynamically allocated
    return result;
  }

  setBaseURL(url);
  fWatchVariableForSyncInterface = 0;
  fTimeoutTask = NULL;  // by default, unless:
  if (timeout > 0) {
    // Schedule a task to be called when the specified timeout interval expires.
    // Note that we do this *before* attempting to send the RTSP command, in case this attempt fails immediately, calling the
    // command response handler - because we want the response handler to unschedule any pending timeout handler.
    fTimeoutTask = envir().taskScheduler().scheduleDelayedTask(timeout*1000000, timeoutHandlerForSyncInterface, this);
  }
  (void)sendDescribeCommand(responseHandlerForSyncInterface, authenticator);

  // Now block (but handling events) until we get a response (or a timeout):
  envir().taskScheduler().doEventLoop(&fWatchVariableForSyncInterface);
  if (fResultCode == 0) return fResultString; // success
  delete[] fResultString;
  return NULL;
}

char* RTSPClient::describeWithPassword(char const* url,
				       char const* username, char const* password,
				       Boolean allowKasennaProtocol, int timeout) {
  Authenticator authenticator(username, password);
  return describeURL(url, &authenticator, allowKasennaProtocol, timeout);
}

char* RTSPClient::sendOptionsCmd(char const* url,
				 char* username, char* password,
				 Authenticator* authenticator,
				 int timeout) {
  char* result = NULL;
  Boolean haveAllocatedAuthenticator = False;
  if (authenticator == NULL) {
    // First, check whether "url" contains a username:password to be used
    // (and no username,password pair was supplied separately):
    if (username == NULL && password == NULL
	&& parseRTSPURLUsernamePassword(url, username, password)) {
      Authenticator newAuthenticator(username,password);
      result = sendOptionsCmd(url, username, password, &newAuthenticator, timeout);
      delete[] username; delete[] password; // they were dynamically allocated
      return result;
    } else if (username != NULL && password != NULL) {
      // Use the separately supplied username and password:
      authenticator = new Authenticator(username,password);
      haveAllocatedAuthenticator = True;

      result = sendOptionsCmd(url, username, password, authenticator, timeout);
      if (result != NULL) { // We are already authorized
	delete authenticator;
	return result;
      }

      // The "realm" field should have been filled in:
      if (authenticator->realm() == NULL) {
	// We haven't been given enough information to try again, so fail:
	delete authenticator;
	return NULL;
      }
    }
  }

  setBaseURL(url);
  fWatchVariableForSyncInterface = 0;
  fTimeoutTask = NULL;  // by default, unless:
  if (timeout > 0) {
    // Schedule a task to be called when the specified timeout interval expires.
    // Note that we do this *before* attempting to send the RTSP command, in case this attempt fails immediately, calling the
    // command response handler - because we want the response handler to unschedule any pending timeout handler.
    fTimeoutTask = envir().taskScheduler().scheduleDelayedTask(timeout*1000000, timeoutHandlerForSyncInterface, this);
  }
  (void)sendOptionsCommand(responseHandlerForSyncInterface, authenticator);
  if (haveAllocatedAuthenticator) delete authenticator;

  // Now block (but handling events) until we get a response (or a timeout):
  envir().taskScheduler().doEventLoop(&fWatchVariableForSyncInterface);
  if (fResultCode == 0) return fResultString; // success
  delete[] fResultString;
  return NULL;
}

Boolean RTSPClient::announceSDPDescription(char const* url,
					   char const* sdpDescription,
					   Authenticator* authenticator,
					   int timeout) {
  setBaseURL(url);
  fWatchVariableForSyncInterface = 0;
  fTimeoutTask = NULL;  // by default, unless:
  if (timeout > 0) {
    // Schedule a task to be called when the specified timeout interval expires.
    // Note that we do this *before* attempting to send the RTSP command, in case this attempt fails immediately, calling the
    // command response handler - because we want the response handler to unschedule any pending timeout handler.
    fTimeoutTask = envir().taskScheduler().scheduleDelayedTask(timeout*1000000, timeoutHandlerForSyncInterface, this);
  }
  (void)sendAnnounceCommand(sdpDescription, responseHandlerForSyncInterface, authenticator);

  // Now block (but handling events) until we get a response (or a timeout):
  envir().taskScheduler().doEventLoop(&fWatchVariableForSyncInterface);
  delete[] fResultString;
  return fResultCode == 0;
}

Boolean RTSPClient
::announceWithPassword(char const* url, char const* sdpDescription,
		       char const* username, char const* password, int timeout) {
  Authenticator authenticator(username,password);
  return announceSDPDescription(url, sdpDescription, &authenticator, timeout);
}

Boolean RTSPClient::setupMediaSubsession(MediaSubsession& subsession,
					 Boolean streamOutgoing,
					 Boolean streamUsingTCP,
					 Boolean forceMulticastOnUnspecified) {
  fWatchVariableForSyncInterface = 0;
  fTimeoutTask = NULL;
  (void)sendSetupCommand(subsession, responseHandlerForSyncInterface, streamOutgoing, streamUsingTCP, forceMulticastOnUnspecified);

  // Now block (but handling events) until we get a response (or a timeout):
  envir().taskScheduler().doEventLoop(&fWatchVariableForSyncInterface);
  delete[] fResultString;
  return fResultCode == 0;
}

Boolean RTSPClient::playMediaSession(MediaSession& session,
				     double start, double end, float scale) {
  fWatchVariableForSyncInterface = 0;
  fTimeoutTask = NULL;
  (void)sendPlayCommand(session, responseHandlerForSyncInterface, start, end, scale);

  // Now block (but handling events) until we get a response (or a timeout):
  envir().taskScheduler().doEventLoop(&fWatchVariableForSyncInterface);
  delete[] fResultString;
  return fResultCode == 0;
}

Boolean RTSPClient::playMediaSubsession(MediaSubsession& subsession,
					double start, double end, float scale,
					Boolean /*hackForDSS*/) {
  // NOTE: The "hackForDSS" flag is no longer supported.  (However, we will consider resupporting it
  // if we get reports that it is still needed.)
  fWatchVariableForSyncInterface = 0;
  fTimeoutTask = NULL;
  (void)sendPlayCommand(subsession, responseHandlerForSyncInterface, start, end, scale);

  // Now block (but handling events) until we get a response (or a timeout):
  envir().taskScheduler().doEventLoop(&fWatchVariableForSyncInterface);
  delete[] fResultString;
  return fResultCode == 0;
}

Boolean RTSPClient::pauseMediaSession(MediaSession& session) {
  fWatchVariableForSyncInterface = 0;
  fTimeoutTask = NULL;
  (void)sendPauseCommand(session, responseHandlerForSyncInterface);

  // Now block (but handling events) until we get a response (or a timeout):
  envir().taskScheduler().doEventLoop(&fWatchVariableForSyncInterface);
  delete[] fResultString;
  return fResultCode == 0;
}

Boolean RTSPClient::pauseMediaSubsession(MediaSubsession& subsession) {
  fWatchVariableForSyncInterface = 0;
  fTimeoutTask = NULL;
  (void)sendPauseCommand(subsession, responseHandlerForSyncInterface);

  // Now block (but handling events) until we get a response (or a timeout):
  envir().taskScheduler().doEventLoop(&fWatchVariableForSyncInterface);
  delete[] fResultString;
  return fResultCode == 0;
}

Boolean RTSPClient::recordMediaSubsession(MediaSubsession& subsession) {
  fWatchVariableForSyncInterface = 0;
  fTimeoutTask = NULL;
  (void)sendRecordCommand(subsession, responseHandlerForSyncInterface);

  // Now block (but handling events) until we get a response (or a timeout):
  envir().taskScheduler().doEventLoop(&fWatchVariableForSyncInterface);
  delete[] fResultString;
  return fResultCode == 0;
}

Boolean RTSPClient::setMediaSessionParameter(MediaSession& session,
					     char const* parameterName,
					     char const* parameterValue) {
  fWatchVariableForSyncInterface = 0;
  fTimeoutTask = NULL;
  (void)sendSetParameterCommand(session, responseHandlerForSyncInterface, parameterName, parameterValue);

  // Now block (but handling events) until we get a response (or a timeout):
  envir().taskScheduler().doEventLoop(&fWatchVariableForSyncInterface);
  delete[] fResultString;
  return fResultCode == 0;
}

Boolean RTSPClient::getMediaSessionParameter(MediaSession& session,
					     char const* parameterName,
					     char*& parameterValue) {
  fWatchVariableForSyncInterface = 0;
  fTimeoutTask = NULL;
  (void)sendGetParameterCommand(session, responseHandlerForSyncInterface, parameterName);

  // Now block (but handling events) until we get a response (or a timeout):
  envir().taskScheduler().doEventLoop(&fWatchVariableForSyncInterface);
  parameterValue = fResultString;
  return fResultCode == 0;
}

Boolean RTSPClient::teardownMediaSession(MediaSession& session) {
  fWatchVariableForSyncInterface = 0;
  fTimeoutTask = NULL;
  (void)sendTeardownCommand(session, NULL);
  return True; // we don't wait for a response to the "TEARDOWN"
}

Boolean RTSPClient::teardownMediaSubsession(MediaSubsession& subsession) {
  fWatchVariableForSyncInterface = 0;
  fTimeoutTask = NULL;
  (void)sendTeardownCommand(subsession, NULL);
  return True; // we don't wait for a response to the "TEARDOWN"
}

Boolean RTSPClient::parseRTSPURLUsernamePassword(char const* url,
						 char*& username,
						 char*& password) {
  username = password = NULL; // by default
  do {
    // Parse the URL as "rtsp://<username>[:<password>]@<whatever>"
    char const* prefix = "rtsp://";
    unsigned const prefixLength = 7;
    if (_strncasecmp(url, prefix, prefixLength) != 0) break;

    // Look for the ':' and '@':
    unsigned usernameIndex = prefixLength;
    unsigned colonIndex = 0, atIndex = 0;
    for (unsigned i = usernameIndex; url[i] != '\0' && url[i] != '/'; ++i) {
      if (url[i] == ':' && colonIndex == 0) {
	colonIndex = i;
      } else if (url[i] == '@') {
	atIndex = i;
	break; // we're done
      }
    }
    if (atIndex == 0) break; // no '@' found

    char* urlCopy = strDup(url);
    urlCopy[atIndex] = '\0';
    if (colonIndex > 0) {
      urlCopy[colonIndex] = '\0';
      password = strDup(&urlCopy[colonIndex+1]);
    } else {
      password = strDup("");
    }
    username = strDup(&urlCopy[usernameIndex]);
    delete[] urlCopy;

    return True;
  } while (0);

  return False;
}

void RTSPClient::responseHandlerForSyncInterface(RTSPClient* rtspClient, int responseCode, char* responseString) {
  if (rtspClient != NULL) rtspClient->responseHandlerForSyncInterface1(responseCode, responseString);
}

void RTSPClient::responseHandlerForSyncInterface1(int responseCode, char* responseString) {
  // If we have a 'timeout task' pending, then unschedule it:
  if (fTimeoutTask != NULL) envir().taskScheduler().unscheduleDelayedTask(fTimeoutTask);

  // Set result values:
  fResultCode = responseCode;
  fResultString = responseString;

  // Signal a break from the event loop (thereby returning from the blocking command):
  fWatchVariableForSyncInterface = ~0;
}

void RTSPClient::timeoutHandlerForSyncInterface(void* rtspClient) {
  if (rtspClient != NULL) ((RTSPClient*)rtspClient)->timeoutHandlerForSyncInterface1();
}

void RTSPClient::timeoutHandlerForSyncInterface1() {
  // A RTSP command has timed out, so we should have a queued request record.  Disable it by setting its response handler to NULL.
  // (Because this is a synchronous interface, there should be exactly one pending response handler - for "fCSeq".)
  // all of them.)
  changeResponseHandler(fCSeq, NULL);
  fTimeoutTask = NULL;

  // Fill in 'negative' return values:
  fResultCode = ~0;
  fResultString = NULL;

  // Signal a break from the event loop (thereby returning from the blocking command):
  fWatchVariableForSyncInterface = ~0;
}

#endif
