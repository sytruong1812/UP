using System;
using System.Linq;
using System.Collections.Generic;

namespace CloudServer
{
    public enum STATES
    {
        CONNECTED,
        REGISTED,
        LOGGED_IN,
        LOGGED_OUT,
        DISCONNECTED
    }

    public struct SessionInfo
    {
        public Guid? SessionId { get; set; }
        public STATES? States { get; set; }
        public int? UserID { get; set; }
        public string UserName { get; set; }
        public string TokenID { get; set; }
        public DateTime? ExpirationTime { get; set; }
    }

    sealed class UserManager
    {
        private UserManager() { }
        public static UserManager Instance { get { return lazy.Value; } }
        private static readonly Lazy<UserManager> lazy = new Lazy<UserManager>(() => new UserManager());
        private static List<SessionInfo> sessions = new List<SessionInfo>();

        public void PrepareSession(Guid sessionId, STATES state)
        {
            var userSession = new SessionInfo
            {
                States = state,
                SessionId = sessionId,
                UserID = null,
                UserName = null,
                TokenID = null,
                ExpirationTime = null,
            };

            sessions.Add(userSession);
        }

        public void AddUserSession(Guid sessionId, STATES state, int userId, string userName)
        {
            var userSession = sessions.FirstOrDefault(s => s.SessionId == sessionId);
            var index = sessions.IndexOf(userSession);
            if (userSession.SessionId.HasValue)
            {
                userSession.UserID = userId;
                userSession.UserName = userName;
                userSession.States = state;

                sessions[index] = userSession;
            }
        }
        public void AddUserSession(Guid sessionId, STATES state, int userId, string userName, string tokenId, TimeSpan expDuration)
        {
            var expirationTime = DateTime.UtcNow.Add(expDuration);
            var userSession = sessions.FirstOrDefault(s => s.SessionId == sessionId);
            var index = sessions.IndexOf(userSession);
            if (userSession.SessionId.HasValue)
            {
                userSession.UserID = userId;
                userSession.TokenID = tokenId;
                userSession.UserName = userName;
                userSession.States = state;
                userSession.ExpirationTime = expirationTime;

                sessions[index] = userSession;
            }
        }

        public void UpdateUserSession(Guid sessionId, STATES state, int userId, string userName)
        {
            var userSession = sessions.FirstOrDefault(s => s.SessionId == sessionId);
            var index = sessions.IndexOf(userSession);
            if (userSession.SessionId.HasValue)
            {
                userSession.States = state;
                userSession.UserID = userId;
                userSession.UserName = userName;
                sessions[index] = userSession;
            }
        }
        public void UpdateUserSession(Guid sessionId, STATES state, string tokenId, TimeSpan expDuration)
        {
            var expirationTime = DateTime.UtcNow.Add(expDuration);
            var userSession = sessions.FirstOrDefault(s => s.SessionId == sessionId);
            var index = sessions.IndexOf(userSession);
            if (userSession.SessionId.HasValue)
            {
                userSession.States = state;
                userSession.TokenID = tokenId;
                userSession.ExpirationTime = expirationTime;
                sessions[index] = userSession;
            }
        }
        public void UpdateUserSession(Guid sessionId, STATES state)
        {
            var userSession = sessions.FirstOrDefault(s => s.SessionId == sessionId);
            var index = sessions.IndexOf(userSession);
            if (userSession.SessionId.HasValue)
            {
                userSession.States = state;
                sessions[index] = userSession;
            }
        }

        public void RemoveSession(Guid sessionId)
        {
            var userSession = sessions.FirstOrDefault(s => s.SessionId == sessionId);
            if (userSession.SessionId.HasValue)
            {
                sessions.Remove(userSession);
            }
        }
        public void RemoveSession(int userId)
        {
            var userSession = sessions.FirstOrDefault(s => s.UserID == userId);
            if (userSession.SessionId.HasValue)
            {
                sessions.Remove(userSession);
            }
        }

        public int? GetUserID(Guid sessionId)
        {
            var userSession = sessions.FirstOrDefault(s => s.SessionId == sessionId);
            if (userSession.SessionId.HasValue)
            {
                if (DateTime.UtcNow < userSession.ExpirationTime)
                {
                    return (int)userSession.UserID;
                }
                else
                {
                    RemoveSession(sessionId);
                }
            }
            return null;
        }
        public int? GetUserID(string tokenId)
        {
            var userSession = sessions.FirstOrDefault(s => s.TokenID == tokenId);
            if (userSession.SessionId.HasValue)
            {
                if (DateTime.UtcNow < userSession.ExpirationTime)
                {
                    return (int)userSession.UserID;
                }
                else
                {
                    RemoveSession((Guid)userSession.SessionId);
                }
            }
            return null;
        }

        public STATES? GetUserStatus(Guid sessionId)
        {
            var userSession = sessions.FirstOrDefault(s => s.SessionId == sessionId);
            if (userSession.SessionId.HasValue)
            {
                if (DateTime.UtcNow < userSession.ExpirationTime)
                {
                    return (STATES)userSession.States;
                }
                else
                {
                    RemoveSession(sessionId);
                    return STATES.LOGGED_OUT;
                }
            }
            return null;
        }
        public STATES? GetUserStatus(int userId)
        {
            var userSession = sessions.FirstOrDefault(s => s.UserID == userId);
            if (userSession.SessionId.HasValue)
            {
                if (DateTime.UtcNow < userSession.ExpirationTime)
                {
                    return (STATES)userSession.States;
                }
                else
                {
                    RemoveSession((Guid)userSession.SessionId);
                    return STATES.LOGGED_OUT;
                }
            }
            return null;
        }
    }
}