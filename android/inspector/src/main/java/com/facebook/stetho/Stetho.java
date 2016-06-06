/*
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

package com.facebook.stetho;

import android.app.Application;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Build;
import com.facebook.stetho.common.LogUtil;
import com.facebook.stetho.common.Util;
import com.facebook.stetho.dumpapp.DumpappHttpSocketLikeHandler;
import com.facebook.stetho.dumpapp.DumpappSocketLikeHandler;
import com.facebook.stetho.dumpapp.Dumper;
import com.facebook.stetho.dumpapp.DumperPlugin;
import com.facebook.stetho.dumpapp.plugins.CrashDumperPlugin;
import com.facebook.stetho.dumpapp.plugins.FilesDumperPlugin;
import com.facebook.stetho.dumpapp.plugins.HprofDumperPlugin;
import com.facebook.stetho.dumpapp.plugins.SharedPreferencesDumperPlugin;
import com.facebook.stetho.inspector.DevtoolsSocketHandler;
import com.facebook.stetho.inspector.console.RuntimeReplFactory;
import com.facebook.stetho.inspector.database.DatabaseFilesProvider;
import com.facebook.stetho.inspector.database.DefaultDatabaseFilesProvider;
import com.facebook.stetho.inspector.database.SqliteDatabaseDriver;
import com.facebook.stetho.inspector.elements.Document;
import com.facebook.stetho.inspector.elements.DocumentProviderFactory;
import com.facebook.stetho.inspector.elements.android.ActivityTracker;
import com.facebook.stetho.inspector.elements.android.AndroidDocumentConstants;
import com.facebook.stetho.inspector.elements.android.AndroidDocumentProviderFactory;
import com.facebook.stetho.inspector.protocol.ChromeDevtoolsDomain;
import com.facebook.stetho.inspector.protocol.module.CSS;
import com.facebook.stetho.inspector.protocol.module.Console;
import com.facebook.stetho.inspector.protocol.module.DOM;
import com.facebook.stetho.inspector.protocol.module.DOMStorage;
import com.facebook.stetho.inspector.protocol.module.Database;
import com.facebook.stetho.inspector.protocol.module.DatabaseConstants;
import com.facebook.stetho.inspector.protocol.module.Debugger;
import com.facebook.stetho.inspector.protocol.module.HeapProfiler;
import com.facebook.stetho.inspector.protocol.module.Inspector;
import com.facebook.stetho.inspector.protocol.module.Network;
import com.facebook.stetho.inspector.protocol.module.Page;
import com.facebook.stetho.inspector.protocol.module.Profiler;
import com.facebook.stetho.inspector.protocol.module.Runtime;
import com.facebook.stetho.inspector.protocol.module.Worker;
import com.facebook.stetho.inspector.runtime.RhinoDetectingRuntimeReplFactory;
import com.facebook.stetho.server.AddressNameHelper;
import com.facebook.stetho.server.LazySocketHandler;
import com.facebook.stetho.server.LocalSocketServer;
import com.facebook.stetho.server.ServerManager;
import com.facebook.stetho.server.ProtocolDetectingSocketHandler;
import com.facebook.stetho.server.SocketHandler;
import com.facebook.stetho.server.SocketHandlerFactory;

import javax.annotation.Nullable;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * Initialization and configuration entry point for the Stetho debugging system.  Simple usage with
 * default plugins and features enabled:
 * <p />
 * <pre>
 *   Stetho.initializeWithDefaults(context)
 * </pre>
 * <p />
 * For more advanced configuration, see {@link #newInitializerBuilder(Context)} or
 * the {@code stetho-sample} for more information.
 */
public class Stetho {
  private Stetho() {
  }

  /**
   * Construct a simple initializer helper which allows you to customize stetho behaviour
   * with additional features, plugins, etc.  See {@link DefaultDumperPluginsBuilder} and
   * {@link DefaultInspectorModulesBuilder} for more information.
   * <p />
   * For simple use cases, consider {@link #initializeWithDefaults(Context)}.
   */
  public static InitializerBuilder newInitializerBuilder(Context context) {
    return new InitializerBuilder(context);
  }

  /**
   * Start the listening server.  Most of the heavy lifting initialization is deferred until the
   * first socket connection is received, allowing this to be safely used for debug builds on
   * even low-end hardware without noticeably affecting performance.
   */
  public static void initializeWithDefaults(final Context context) {
    initialize(new Initializer(context) {
      @Override
      protected Iterable<DumperPlugin> getDumperPlugins() {
        return new DefaultDumperPluginsBuilder(context).finish();
      }

      @Override
      protected Iterable<ChromeDevtoolsDomain> getInspectorModules() {
        return new DefaultInspectorModulesBuilder(context).finish();
      }
    });
  }

  /**
   * Start the listening service, providing a custom initializer as per
   * {@link #newInitializerBuilder}.
   *
   * @see #initializeWithDefaults(Context)
   */
  public static void initialize(final Initializer initializer) {
    // Hook activity tracking so that after Stetho is attached we can figure out what
    // activities are present.
    boolean isTrackingActivities = ActivityTracker.get().beginTrackingIfPossible(
        (Application)initializer.mContext.getApplicationContext());
    if (!isTrackingActivities) {
      LogUtil.w("Automatic activity tracking not available on this API level, caller must invoke " +
          "ActivityTracker methods manually!");
    }

    initializer.start();
  }

  public static DumperPluginsProvider defaultDumperPluginsProvider(final Context context) {
    return new DumperPluginsProvider() {
      @Override
      public Iterable<DumperPlugin> get() {
        return new DefaultDumperPluginsBuilder(context).finish();
      }
    };
  }

  public static InspectorModulesProvider defaultInspectorModulesProvider(final Context context) {
    return new InspectorModulesProvider() {
      @Override
      public Iterable<ChromeDevtoolsDomain> get() {
        return new DefaultInspectorModulesBuilder(context).finish();
      }
    };
  }

  private static class PluginBuilder<T> {
    private final Set<String> mProvidedNames = new HashSet<>();
    private final Set<String> mRemovedNames = new HashSet<>();

    private final ArrayList<T> mPlugins = new ArrayList<>();

    private boolean mFinished;

    public void provide(String name, T plugin) {
      throwIfFinished();
      mPlugins.add(plugin);
      mProvidedNames.add(name);
    }

    public void provideIfDesired(String name, T plugin) {
      throwIfFinished();
      if (!mRemovedNames.contains(name)) {
        if (mProvidedNames.add(name)) {
          mPlugins.add(plugin);
        }
      }
    }

    public void remove(String pluginName) {
      throwIfFinished();
      mRemovedNames.remove(pluginName);
    }

    private void throwIfFinished() {
      if (mFinished) {
        throw new IllegalStateException("Must not continue to build after finish()");
      }
    }

    public Iterable<T> finish() {
      mFinished = true;
      return mPlugins;
    }
  }

  /**
   * Convenience mechanism to extend the default set of dumper plugins provided by Stetho.
   *
   * @see #initializeWithDefaults(Context)
   */
  public static final class DefaultDumperPluginsBuilder {
    private final Context mContext;
    private final PluginBuilder<DumperPlugin> mDelegate = new PluginBuilder<>();

    public DefaultDumperPluginsBuilder(Context context) {
      mContext = context;
    }

    public DefaultDumperPluginsBuilder provide(DumperPlugin plugin) {
      mDelegate.provide(plugin.getName(), plugin);
      return this;
    }

    private DefaultDumperPluginsBuilder provideIfDesired(DumperPlugin plugin) {
      mDelegate.provideIfDesired(plugin.getName(), plugin);
      return this;
    }

    public DefaultDumperPluginsBuilder remove(String pluginName) {
      mDelegate.remove(pluginName);
      return this;
    }

    public Iterable<DumperPlugin> finish() {
      provideIfDesired(new HprofDumperPlugin(mContext));
      provideIfDesired(new SharedPreferencesDumperPlugin(mContext));
      provideIfDesired(new CrashDumperPlugin());
      provideIfDesired(new FilesDumperPlugin(mContext));
      return mDelegate.finish();
    }
  }

  /**
   * Configuration mechanism to customize the behaviour of the standard set of inspector
   * modules satisfying the Chrome DevTools protocol.  Note that while it is still technically
   * possible to manually control these modules, this API is strongly discouraged and will not
   * necessarily be supported in future releases.
   */
  public static final class DefaultInspectorModulesBuilder {
    private final Application mContext;
    private final PluginBuilder<ChromeDevtoolsDomain> mDelegate = new PluginBuilder<>();

    @Nullable private DocumentProviderFactory mDocumentProvider;
    @Nullable private RuntimeReplFactory mRuntimeRepl;
    @Nullable private DatabaseFilesProvider mDatabaseFilesProvider;
    @Nullable private List<Database.DatabaseDriver> mDatabaseDrivers;

    public DefaultInspectorModulesBuilder(Context context) {
      mContext = (Application)context.getApplicationContext();
    }

    /**
     * Provide a custom document provider factory which can operate on the logical DOM exposed to
     * Chrome in the Elements tab.  An Android View hierarchy instance is provided by
     * default if this method is not called.
     * <p />
     * <i>Experimental.</i>  This API may be changed or removed in the future.
     */
    public DefaultInspectorModulesBuilder documentProvider(DocumentProviderFactory factory) {
      mDocumentProvider = factory;
      return this;
    }

    /**
     * Provide a custom runtime REPL (read-eval-print loop) implementation for the Console tab.
     * By default an implementation will be provided for you that automatically detects
     * the existence of {@code stetho-js-rhino} (Mozilla's Rhino engine) and uses it if available.
     * <p />
     * To customize the Rhino implementation, see {@code stetho-js-rhino} documentation.
     */
    public DefaultInspectorModulesBuilder runtimeRepl(RuntimeReplFactory factory) {
      mRuntimeRepl = factory;
      return this;
    }

    /**
     * Customize the location of database files that Stetho will propogate in the UI.  Android's
     * {@link Context#getDatabasePath} method will be used by default if not overridden here.
     */
    public DefaultInspectorModulesBuilder databaseFiles(DatabaseFilesProvider provider) {
      mDatabaseFilesProvider = provider;
      return this;
    }

    /**
     * Extend and provide additional database drivers. Currently two database drivers are supported
     * in this lib: <br>
     *   1. <code>SqliteDatabaseDriver</code> - Presents sqlite databases and all tables of the app.<br>
     *   2. <code>ContentProviderDatabaseDriver</code> - Configure and present content providers data.
     */
    public DefaultInspectorModulesBuilder provideDatabaseDriver(Database.DatabaseDriver databaseDriver) {
      if (mDatabaseDrivers == null) {
        mDatabaseDrivers = new ArrayList<>();
      }
      mDatabaseDrivers.add(databaseDriver);
      return this;
    }

    /**
     * Provide either a new domain module or override an existing one.
     *
     * @deprecated This fine-grained control of the devtools modules is no longer supportable
     *     given the lack of isolation of modules in the actual protocol (many cross dependencies
     *     emerge when you implement more and more of the real protocol).
     */
    @Deprecated
    public DefaultInspectorModulesBuilder provide(ChromeDevtoolsDomain module) {
      mDelegate.provide(module.getClass().getName(), module);
      return this;
    }

    private DefaultInspectorModulesBuilder provideIfDesired(ChromeDevtoolsDomain module) {
      mDelegate.provideIfDesired(module.getClass().getName(), module);
      return this;
    }

    /**
     * Remove an existing domain module.
     *
     * @deprecated This fine-grained control of the devtools modules is no longer supportable
     *     given the lack of isolation of modules in the actual protocol (many cross dependencies
     *     emerge when you implement more and more of the real protocol).
     */
    @Deprecated
    public DefaultInspectorModulesBuilder remove(String moduleName) {
      mDelegate.remove(moduleName);
      return this;
    }

    public Iterable<ChromeDevtoolsDomain> finish() {
      provideIfDesired(new Console());
      provideIfDesired(new Debugger());
      DocumentProviderFactory documentModel = resolveDocumentProvider();
      if (documentModel != null) {
        Document document = new Document(documentModel);
        provideIfDesired(new DOM(document));
        provideIfDesired(new CSS(document));
      }
      provideIfDesired(new DOMStorage(mContext));
      provideIfDesired(new HeapProfiler());
      provideIfDesired(new Inspector());
      provideIfDesired(new Network(mContext));
      provideIfDesired(new Page(mContext));
      provideIfDesired(new Profiler());
      provideIfDesired(
          new Runtime(
              mRuntimeRepl != null ?
              mRuntimeRepl :
              new RhinoDetectingRuntimeReplFactory(mContext)));
      provideIfDesired(new Worker());
      if (Build.VERSION.SDK_INT >= DatabaseConstants.MIN_API_LEVEL) {
        Database database = new Database();
        database.add(new SqliteDatabaseDriver(mContext,
            mDatabaseFilesProvider != null ?
                mDatabaseFilesProvider :
                new DefaultDatabaseFilesProvider(mContext)));
        if (mDatabaseDrivers != null) {
          for (Database.DatabaseDriver databaseDriver : mDatabaseDrivers) {
            database.add(databaseDriver);
          }
        }
        provideIfDesired(database);
      }
      return mDelegate.finish();
    }

    @Nullable
    private DocumentProviderFactory resolveDocumentProvider() {
      if (mDocumentProvider != null) {
        return mDocumentProvider;
      }
      if (Build.VERSION.SDK_INT >= AndroidDocumentConstants.MIN_API_LEVEL) {
        return new AndroidDocumentProviderFactory(mContext);
      }
      return null;
    }
  }

  /**
   * Callers can choose to subclass this directly to provide the initialization configuration
   * or they can construct a concrete instance using {@link #newInitializerBuilder(Context)}.
   */
  public static abstract class Initializer {
    private final Context mContext;

    protected Initializer(Context context) {
      mContext = context.getApplicationContext();
    }

    @Nullable
    protected abstract Iterable<DumperPlugin> getDumperPlugins();

    @Nullable
    protected abstract Iterable<ChromeDevtoolsDomain> getInspectorModules();

    final void start() {
      // Note that _devtools_remote is a magic suffix understood by Chrome which causes
      // the discovery process to begin.
      LocalSocketServer server = new LocalSocketServer(
          "main",
          AddressNameHelper.createCustomAddress("_devtools_remote"),
          new LazySocketHandler(new RealSocketHandlerFactory()));

      ServerManager serverManager = new ServerManager(server);
      serverManager.start();
    }

    private class RealSocketHandlerFactory implements SocketHandlerFactory {
      @Override
      public SocketHandler create() {
        ProtocolDetectingSocketHandler socketHandler =
            new ProtocolDetectingSocketHandler(mContext);

        Iterable<DumperPlugin> dumperPlugins = getDumperPlugins();
        if (dumperPlugins != null) {
          Dumper dumper = new Dumper(dumperPlugins);

          socketHandler.addHandler(
              new ProtocolDetectingSocketHandler.ExactMagicMatcher(
                  DumpappSocketLikeHandler.PROTOCOL_MAGIC),
              new DumpappSocketLikeHandler(dumper));

          // Support the old HTTP-based protocol since it's relatively straight forward to do.
          DumpappHttpSocketLikeHandler legacyHandler = new DumpappHttpSocketLikeHandler(dumper);
          socketHandler.addHandler(
              new ProtocolDetectingSocketHandler.ExactMagicMatcher(
                  "GET /dumpapp".getBytes()),
              legacyHandler);
          socketHandler.addHandler(
              new ProtocolDetectingSocketHandler.ExactMagicMatcher(
                  "POST /dumpapp".getBytes()),
              legacyHandler);
        }

        Iterable<ChromeDevtoolsDomain> inspectorModules = getInspectorModules();
        if (inspectorModules != null) {
          socketHandler.addHandler(
              new ProtocolDetectingSocketHandler.AlwaysMatchMatcher(),
              new DevtoolsSocketHandler(mContext, inspectorModules));
        }

        return socketHandler;
      }
    }
  }

  /**
   * Configure what services are to be enabled in this instance of Stetho.
   */
  public static class InitializerBuilder {
    final Context mContext;

    @Nullable DumperPluginsProvider mDumperPlugins;
    @Nullable InspectorModulesProvider mInspectorModules;

    private InitializerBuilder(Context context) {
      mContext = context.getApplicationContext();
    }

    /**
     * Enable use of the {@code dumpapp} system.  This is an extension to Stetho which allows
     * developers to configure custom debug endpoints as tiny programs embedded inside of a larger
     * running Android application.  Examples of this would be simple utilities to visualize and
     * edit {@link SharedPreferences} data, kick off sync or other background tasks, inject custom
     * data temporarily into the process for debugging/reproducibility, upload error reports,
     * etc.
     * <p>
     * See {@code ./scripts/dumpapp} for more information on how to use this system once
     * enabled.
     *
     * @param plugins The set of plugins to use.
     */
    public InitializerBuilder enableDumpapp(DumperPluginsProvider plugins) {
      mDumperPlugins = Util.throwIfNull(plugins);
      return this;
    }

    public InitializerBuilder enableWebKitInspector(InspectorModulesProvider modules) {
      mInspectorModules = modules;
      return this;
    }

    public Initializer build() {
      return new BuilderBasedInitializer(this);
    }
  }

  private static class BuilderBasedInitializer extends Initializer {
    @Nullable private final DumperPluginsProvider mDumperPlugins;
    @Nullable private final InspectorModulesProvider mInspectorModules;

    private BuilderBasedInitializer(InitializerBuilder b) {
      super(b.mContext);
      mDumperPlugins = b.mDumperPlugins;
      mInspectorModules = b.mInspectorModules;
    }

    @Nullable
    @Override
    protected Iterable<DumperPlugin> getDumperPlugins() {
      return mDumperPlugins != null ? mDumperPlugins.get() : null;
    }

    @Nullable
    @Override
    protected Iterable<ChromeDevtoolsDomain> getInspectorModules() {
      return mInspectorModules != null ? mInspectorModules.get() : null;
    }
  }
}
