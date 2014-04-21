# -*- coding: utf-8 -*- 

###########################################################################
## Python code generated with wxFormBuilder (version Nov  6 2013)
## http://www.wxformbuilder.org/
##
## PLEASE DO "NOT" EDIT THIS FILE!
###########################################################################

from wx.lib.bmp_button import BmpButton
from wx.lib.text_button import TextButton
from wx.lib.arrow_button import ArrowButton
import wx
import wx.xrc

###########################################################################
## Class DownloadDialog
###########################################################################

class DownloadDialog ( wx.Dialog ):
	
	def __init__( self, parent ):
		wx.Dialog.__init__ ( self, parent, id = wx.ID_ANY, title = u"Downloader", pos = wx.DefaultPosition, size = wx.DefaultSize, style = wx.DEFAULT_DIALOG_STYLE|wx.RESIZE_BORDER|wx.CLIP_CHILDREN )
		
		self.SetSizeHintsSz( wx.DefaultSize, wx.DefaultSize )
		
		bSizer5 = wx.BoxSizer( wx.VERTICAL )
		
		self.splitter = wx.SplitterWindow( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.SP_3D )
		self.splitter.Bind( wx.EVT_IDLE, self.splitterOnIdle )
		
		self.panel = wx.Panel( self.splitter, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		self.panel.SetBackgroundColour( wx.Colour( 255, 255, 255 ) )
		
		bSizer13 = wx.BoxSizer( wx.HORIZONTAL )
		
		bSizer1 = wx.BoxSizer( wx.VERTICAL )
		
		bSizer2 = wx.BoxSizer( wx.HORIZONTAL )
		
		self.prev_week = BmpButton( self.panel, wx.ID_ANY, wx.ICON( download.images.prev_week ), wx.DefaultPosition, wx.DefaultSize, 0 )
		self.prev_week.SetToolTipString( u"Previous Week" )
		
		bSizer2.Add( self.prev_week, 0, wx.RIGHT|wx.ALIGN_CENTER_VERTICAL, 10 )
		
		self.prev = BmpButton( self.panel, wx.ID_ANY, wx.ICON( download.images.prev ), wx.DefaultPosition, wx.DefaultSize, 0 )
		self.prev.SetToolTipString( u"Previous Day" )
		
		bSizer2.Add( self.prev, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.label = TextButton( self.panel, wx.ID_ANY, u"Label", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.label.SetFont( wx.Font( 12, 70, 90, 92, False, wx.EmptyString ) )
		self.label.SetForegroundColour( wx.Colour( 0, 0, 255 ) )
		self.label.SetToolTipString( u"Download missing puzzles" )
		
		bSizer2.Add( self.label, 1, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.next = BmpButton( self.panel, wx.ID_ANY, wx.ICON( download.images.next ), wx.DefaultPosition, wx.DefaultSize, 0 )
		self.next.SetToolTipString( u"Next Day" )
		
		bSizer2.Add( self.next, 0, wx.ALIGN_CENTER_VERTICAL, 10 )
		
		self.next_week = BmpButton( self.panel, wx.ID_ANY, wx.ICON( download.images.next_week ), wx.DefaultPosition, wx.DefaultSize, 0 )
		self.next_week.SetToolTipString( u"Next Week" )
		
		bSizer2.Add( self.next_week, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT, 10 )
		
		
		bSizer1.Add( bSizer2, 0, wx.EXPAND|wx.ALL, 5 )
		
		bSizer3 = wx.BoxSizer( wx.HORIZONTAL )
		
		kindChoices = [ u"Day", u"Week", u"Month" ]
		self.kind = wx.Choice( self.panel, wx.ID_ANY, wx.DefaultPosition, wx.Size( 80,-1 ), kindChoices, 0 )
		self.kind.SetSelection( 0 )
		bSizer3.Add( self.kind, 0, 0, 5 )
		
		
		bSizer3.AddSpacer( ( 0, 0), 1, wx.EXPAND, 5 )
		
		queueCountSizer = wx.BoxSizer( wx.HORIZONTAL )
		
		self.queue_count = TextButton( self.panel, wx.ID_ANY, u"0", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.queue_count.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), 70, 90, 92, False, wx.EmptyString ) )
		self.queue_count.SetForegroundColour( wx.Colour( 0, 0, 255 ) )
		self.queue_count.SetToolTipString( u"Download queue" )
		
		queueCountSizer.Add( self.queue_count, 0, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 5 )
		
		self.queueBmp = wx.StaticBitmap( self.panel, wx.ID_ANY, wx.ICON( download.images.down ), wx.DefaultPosition, wx.DefaultSize, 0 )
		queueCountSizer.Add( self.queueBmp, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		
		bSizer3.Add( queueCountSizer, 0, wx.EXPAND|wx.ALIGN_CENTER_VERTICAL|wx.RIGHT|wx.LEFT, 5 )
		
		errorCountSizer = wx.BoxSizer( wx.HORIZONTAL )
		
		self.error_count = TextButton( self.panel, wx.ID_ANY, u"0", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.error_count.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), 70, 90, 92, False, wx.EmptyString ) )
		self.error_count.SetForegroundColour( wx.Colour( 0, 0, 255 ) )
		self.error_count.SetToolTipString( u"Download missing puzzles" )
		
		errorCountSizer.Add( self.error_count, 0, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 5 )
		
		self.errorBmp = wx.StaticBitmap( self.panel, wx.ID_ANY, wx.ICON( download.images.error ), wx.DefaultPosition, wx.DefaultSize, 0 )
		errorCountSizer.Add( self.errorBmp, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		
		bSizer3.Add( errorCountSizer, 0, wx.EXPAND, 5 )
		
		
		bSizer3.AddSpacer( ( 0, 0), 1, wx.EXPAND, 5 )
		
		self.date = wx.DatePickerCtrl( self.panel, wx.ID_ANY, wx.DefaultDateTime, wx.DefaultPosition, wx.Size( 80,-1 ), wx.DP_DROPDOWN|wx.DP_SHOWCENTURY )
		bSizer3.Add( self.date, 0, 0, 5 )
		
		
		bSizer1.Add( bSizer3, 0, wx.EXPAND|wx.BOTTOM|wx.RIGHT|wx.LEFT, 5 )
		
		self.scroller = wx.ScrolledWindow( self.panel, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.DOUBLE_BORDER|wx.HSCROLL|wx.VSCROLL )
		self.scroller.SetScrollRate( 10, 10 )
		bSizer8 = wx.BoxSizer( wx.VERTICAL )
		
		
		self.scroller.SetSizer( bSizer8 )
		self.scroller.Layout()
		bSizer8.Fit( self.scroller )
		bSizer1.Add( self.scroller, 1, wx.EXPAND, 5 )
		
		
		bSizer13.Add( bSizer1, 1, wx.EXPAND, 5 )
		
		
		self.panel.SetSizer( bSizer13 )
		self.panel.Layout()
		bSizer13.Fit( self.panel )
		self.splitter.Initialize( self.panel )
		bSizer5.Add( self.splitter, 1, wx.EXPAND, 5 )
		
		
		self.SetSizer( bSizer5 )
		self.Layout()
		bSizer5.Fit( self )
		
		self.Centre( wx.BOTH )
		
		# Connect Events
		self.Bind( wx.EVT_CLOSE, self.OnClose )
		self.prev_week.Bind( wx.EVT_BUTTON, self.OnPrevWeek )
		self.prev.Bind( wx.EVT_BUTTON, self.OnPrev )
		self.label.Bind( wx.EVT_BUTTON, self.OnDownloadMissing )
		self.next.Bind( wx.EVT_BUTTON, self.OnNext )
		self.next_week.Bind( wx.EVT_BUTTON, self.OnNextWeek )
		self.kind.Bind( wx.EVT_CHOICE, self.OnKindChanged )
		self.queue_count.Bind( wx.EVT_BUTTON, self.OnQueueClick )
		self.error_count.Bind( wx.EVT_BUTTON, self.OnErrorClick )
		self.date.Bind( wx.EVT_DATE_CHANGED, self.OnDateChanged )
	
	def __del__( self ):
		pass
	
	
	# Virtual event handlers, overide them in your derived class
	def OnClose( self, event ):
		event.Skip()
	
	def OnPrevWeek( self, event ):
		event.Skip()
	
	def OnPrev( self, event ):
		event.Skip()
	
	def OnDownloadMissing( self, event ):
		event.Skip()
	
	def OnNext( self, event ):
		event.Skip()
	
	def OnNextWeek( self, event ):
		event.Skip()
	
	def OnKindChanged( self, event ):
		event.Skip()
	
	def OnQueueClick( self, event ):
		event.Skip()
	
	def OnErrorClick( self, event ):
		event.Skip()
	
	def OnDateChanged( self, event ):
		event.Skip()
	
	def splitterOnIdle( self, event ):
		self.splitter.SetSashPosition( 0 )
		self.splitter.Unbind( wx.EVT_IDLE )
	

###########################################################################
## Class DownloadHeading
###########################################################################

class DownloadHeading ( wx.Panel ):
	
	def __init__( self, parent ):
		wx.Panel.__init__ ( self, parent, id = wx.ID_ANY, pos = wx.DefaultPosition, size = wx.Size( -1,-1 ), style = wx.TAB_TRAVERSAL )
		
		bSizer7 = wx.BoxSizer( wx.VERTICAL )
		
		self.m_panel4 = wx.Panel( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		self.m_panel4.SetBackgroundColour( wx.Colour( 240, 240, 240 ) )
		
		bSizer11 = wx.BoxSizer( wx.VERTICAL )
		
		bSizer8 = wx.BoxSizer( wx.HORIZONTAL )
		
		self.expand_button = ArrowButton( self.m_panel4, wx.ID_ANY, u">>", wx.DefaultPosition, wx.DefaultSize, 0 )
		bSizer8.Add( self.expand_button, 0, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 5 )
		
		self.label = TextButton( self.m_panel4, wx.ID_ANY, u"Source", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.label.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), 70, 90, 92, False, wx.EmptyString ) )
		
		bSizer8.Add( self.label, 1, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 5 )
		
		self.download_button = TextButton( self.m_panel4, wx.ID_ANY, u"0 Puzzles", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.download_button.SetToolTipString( u"Download missing puzzles" )
		
		bSizer8.Add( self.download_button, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		
		bSizer11.Add( bSizer8, 1, wx.EXPAND|wx.ALL, 2 )
		
		
		self.m_panel4.SetSizer( bSizer11 )
		self.m_panel4.Layout()
		bSizer11.Fit( self.m_panel4 )
		bSizer7.Add( self.m_panel4, 1, wx.EXPAND, 5 )
		
		self.m_staticline5 = wx.StaticLine( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.LI_HORIZONTAL )
		bSizer7.Add( self.m_staticline5, 0, wx.EXPAND, 5 )
		
		self.line = wx.StaticLine( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.LI_HORIZONTAL )
		bSizer7.Add( self.line, 0, wx.EXPAND, 5 )
		
		
		self.SetSizer( bSizer7 )
		self.Layout()
		bSizer7.Fit( self )
		
		# Connect Events
		self.expand_button.Bind( wx.EVT_BUTTON, self.OnExpand )
		self.label.Bind( wx.EVT_BUTTON, self.OnLabelClick )
		self.download_button.Bind( wx.EVT_BUTTON, self.OnDownloadMissing )
	
	def __del__( self ):
		pass
	
	
	# Virtual event handlers, overide them in your derived class
	def OnExpand( self, event ):
		event.Skip()
	
	def OnLabelClick( self, event ):
		event.Skip()
	
	def OnDownloadMissing( self, event ):
		event.Skip()
	

###########################################################################
## Class DownloadList
###########################################################################

class DownloadList ( wx.Panel ):
	
	def __init__( self, parent ):
		wx.Panel.__init__ ( self, parent, id = wx.ID_ANY, pos = wx.DefaultPosition, size = wx.Size( -1,-1 ), style = wx.TAB_TRAVERSAL )
		
		bSizer10 = wx.BoxSizer( wx.VERTICAL )
		
		bSizer13 = wx.BoxSizer( wx.HORIZONTAL )
		
		self.label = wx.StaticText( self, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		self.label.Wrap( -1 )
		bSizer13.Add( self.label, 0, wx.ALL, 5 )
		
		
		bSizer13.AddSpacer( ( 0, 0), 1, wx.EXPAND, 5 )
		
		self.erase_history = TextButton( self, wx.ID_ANY, u"Clear History", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.erase_history.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), 70, 90, 92, False, wx.EmptyString ) )
		self.erase_history.SetForegroundColour( wx.Colour( 0, 0, 255 ) )
		self.erase_history.SetToolTipString( u"Download missing puzzles" )
		
		bSizer13.Add( self.erase_history, 0, wx.ALL, 5 )
		
		self.cancel_downloads = TextButton( self, wx.ID_ANY, u"Cancel Downloads", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.cancel_downloads.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), 70, 90, 92, False, wx.EmptyString ) )
		self.cancel_downloads.SetForegroundColour( wx.Colour( 0, 0, 255 ) )
		self.cancel_downloads.SetToolTipString( u"Download missing puzzles" )
		
		bSizer13.Add( self.cancel_downloads, 0, wx.ALL, 5 )
		
		
		bSizer10.Add( bSizer13, 0, wx.EXPAND, 5 )
		
		self.list = wx.LuaListCtrl( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.LC_REPORT|wx.LC_VIRTUAL )
		bSizer10.Add( self.list, 1, wx.ALL|wx.EXPAND, 5 )
		
		
		self.SetSizer( bSizer10 )
		self.Layout()
		bSizer10.Fit( self )
		
		# Connect Events
		self.erase_history.Bind( wx.EVT_BUTTON, self.OnClearHistory )
		self.cancel_downloads.Bind( wx.EVT_BUTTON, self.OnCancelDownloads )
		self.list.Bind( wx.EVT_LIST_COL_CLICK, self.OnHeaderClick )
	
	def __del__( self ):
		pass
	
	
	# Virtual event handlers, overide them in your derived class
	def OnClearHistory( self, event ):
		event.Skip()
	
	def OnCancelDownloads( self, event ):
		event.Skip()
	
	def OnHeaderClick( self, event ):
		event.Skip()
	

###########################################################################
## Class PromptDialog
###########################################################################

class PromptDialog ( wx.Dialog ):
	
	def __init__( self, parent ):
		wx.Dialog.__init__ ( self, parent, id = wx.ID_ANY, title = u"XWord Message", pos = wx.DefaultPosition, size = wx.DefaultSize, style = wx.DEFAULT_DIALOG_STYLE )
		
		self.SetSizeHintsSz( wx.DefaultSize, wx.DefaultSize )
		
		bSizer15 = wx.BoxSizer( wx.VERTICAL )
		
		bSizer14 = wx.BoxSizer( wx.VERTICAL )
		
		self.message = wx.StaticText( self, wx.ID_ANY, u"Message", wx.DefaultPosition, wx.DefaultSize, wx.ALIGN_LEFT )
		self.message.Wrap( -1 )
		bSizer14.Add( self.message, 0, wx.BOTTOM, 10 )
		
		fgSizer1 = wx.FlexGridSizer( 0, 2, 10, 10 )
		fgSizer1.AddGrowableCol( 1 )
		fgSizer1.SetFlexibleDirection( wx.BOTH )
		fgSizer1.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		
		bSizer14.Add( fgSizer1, 1, wx.EXPAND|wx.RIGHT|wx.LEFT, 5 )
		
		self.m_staticline3 = wx.StaticLine( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.LI_HORIZONTAL )
		bSizer14.Add( self.m_staticline3, 0, wx.EXPAND|wx.TOP|wx.BOTTOM, 10 )
		
		buttons = wx.StdDialogButtonSizer()
		self.buttonsOK = wx.Button( self, wx.ID_OK )
		buttons.AddButton( self.buttonsOK )
		self.buttonsCancel = wx.Button( self, wx.ID_CANCEL )
		buttons.AddButton( self.buttonsCancel )
		buttons.Realize();
		
		bSizer14.Add( buttons, 0, wx.EXPAND|wx.RIGHT|wx.LEFT, 5 )
		
		
		bSizer15.Add( bSizer14, 1, wx.EXPAND|wx.ALL, 10 )
		
		
		self.SetSizer( bSizer15 )
		self.Layout()
		bSizer15.Fit( self )
		
		self.Centre( wx.BOTH )
	
	def __del__( self ):
		pass
	

