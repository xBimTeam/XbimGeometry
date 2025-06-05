using Microsoft.Extensions.Logging;
using System;
using Xbim.Common;

namespace Xbim.ModelGeometry.Scene
{
    public class ICanLog
    {
        protected ILogger _logger;

        internal ICanLog(ILogger logger)
        {
            _logger = logger;
        }

        internal void LogWarning(object entity, string format, params object[] args)
        {
            LogWarning(null, entity, format, args);
        }

        internal void LogWarning(Exception ex, object entity, string format, params object[] args)
        {
            if (_logger != null)
            {
                var msg = String.Format(format, args);
                if (entity is IPersistEntity ifcEntity)
                    _logger.LogWarning(ex, "GeomScene: #{entityLabel}={entityType} [{message}]",
                        ifcEntity.EntityLabel, ifcEntity.GetType().Name, msg);
                else
                    _logger.LogWarning(ex, "GeomScene: {entityType} [{message}]", entity.GetType().Name, msg);
            }
        }

        internal void LogInfo(object entity, string format, params object[] args)
        {

            if (_logger != null)
            {
                var msg = String.Format(format, args);
                if (entity is IPersistEntity ifcEntity)
                    _logger.LogInformation("GeomScene: #{entityLabel}={entityType} [{message}]",
                        ifcEntity.EntityLabel, ifcEntity.GetType().Name, msg);
                else
                    _logger.LogInformation("GeomScene: {entityType} [{message}]", entity.GetType().Name, msg);
            }
        }

        internal void LogError(object entity, string format, params object[] args)
        {
            LogError(null, entity, format, args);
        }

        internal void LogError(Exception ex, object entity, string format, params object[] args)
        {
            if (_logger != null)
            {
                var msg = String.Format(format, args);
                if (entity is IPersistEntity ifcEntity)
                    _logger.LogError(ex, "GeomScene: #{entityLabel}={entityType} [{message}]",
                        ifcEntity.EntityLabel, ifcEntity.GetType().Name, msg);
                else
                    _logger.LogError(ex, "GeomScene: {entityType} [{message}]", entity.GetType().Name, msg);
            }
        }

        internal void LogError(string msg, Exception ex = null)
        {
            if (_logger != null)
            {
                if (ex == null)
                {
                    _logger.LogError(msg);
                }
                else
                {
                    _logger.LogError(ex, msg);
                }

            }
        }
        internal void LogDebug(object entity, string format, params object[] args)
        {
            if (_logger != null)
            {
                var msg = String.Format(format, args);
                if (entity is IPersistEntity ifcEntity)
                    _logger.LogDebug("GeomScene: #{entityLabel}={entityType} [{message}]",
                        ifcEntity.EntityLabel, ifcEntity.GetType().Name, msg);
                else
                    _logger.LogDebug("GeomScene: {entityType} [{message}]", entity.GetType().Name, msg);
            }
        }
    }
}
