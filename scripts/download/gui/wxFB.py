# -*- coding: utf-8 -*- 

###########################################################################
## Python code generated with wxFormBuilder (version Oct 10 2016)
## http://www.wxformbuilder.org/
##
## PLEASE DO "NOT" EDIT THIS FILE!
###########################################################################

from wx.lib.bmp_button import BmpButton
from wx.lib.text_button import TextButton
from wx.lib.arrow_button import ArrowButton
from wx.lib.checklist import CheckListCtrl
from wx.lib.ctrlgrid import ControlGrid
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
		self.label.SetFont( wx.Font( 12, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD, False, wx.EmptyString ) )
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
		self.queue_count.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD, False, wx.EmptyString ) )
		self.queue_count.SetForegroundColour( wx.Colour( 0, 0, 255 ) )
		self.queue_count.SetToolTipString( u"Download queue" )
		
		queueCountSizer.Add( self.queue_count, 0, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 5 )
		
		self.queueBmp = wx.StaticBitmap( self.panel, wx.ID_ANY, wx.ICON( download.images.down ), wx.DefaultPosition, wx.DefaultSize, 0 )
		queueCountSizer.Add( self.queueBmp, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		
		bSizer3.Add( queueCountSizer, 0, wx.EXPAND|wx.RIGHT|wx.LEFT, 5 )
		
		errorCountSizer = wx.BoxSizer( wx.HORIZONTAL )
		
		self.error_count = TextButton( self.panel, wx.ID_ANY, u"0", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.error_count.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD, False, wx.EmptyString ) )
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
		self.label.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD, False, wx.EmptyString ) )
		
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
		self.erase_history.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD, False, wx.EmptyString ) )
		self.erase_history.SetForegroundColour( wx.Colour( 0, 0, 255 ) )
		
		bSizer13.Add( self.erase_history, 0, wx.ALL, 5 )
		
		self.cancel_downloads = TextButton( self, wx.ID_ANY, u"Cancel Downloads", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.cancel_downloads.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD, False, wx.EmptyString ) )
		self.cancel_downloads.SetForegroundColour( wx.Colour( 0, 0, 255 ) )
		
		bSizer13.Add( self.cancel_downloads, 0, wx.ALL, 5 )
		
		self.close = TextButton( self, wx.ID_ANY, u"X", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.close.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD, False, wx.EmptyString ) )
		self.close.SetForegroundColour( wx.Colour( 0, 0, 255 ) )
		self.close.SetToolTipString( u"Close" )
		
		bSizer13.Add( self.close, 0, wx.ALL, 5 )
		
		
		bSizer10.Add( bSizer13, 0, wx.EXPAND, 5 )
		
		self.list = wx.LuaListCtrl( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.LC_REPORT|wx.LC_VIRTUAL )
		bSizer10.Add( self.list, 1, wx.ALL|wx.EXPAND, 5 )
		
		
		self.SetSizer( bSizer10 )
		self.Layout()
		bSizer10.Fit( self )
		
		# Connect Events
		self.erase_history.Bind( wx.EVT_BUTTON, self.OnClearHistory )
		self.cancel_downloads.Bind( wx.EVT_BUTTON, self.OnCancelDownloads )
		self.close.Bind( wx.EVT_BUTTON, self.OnClose )
		self.list.Bind( wx.EVT_LIST_COL_CLICK, self.OnHeaderClick )
	
	def __del__( self ):
		pass
	
	
	# Virtual event handlers, overide them in your derived class
	def OnClearHistory( self, event ):
		event.Skip()
	
	def OnCancelDownloads( self, event ):
		event.Skip()
	
	def OnClose( self, event ):
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
	

###########################################################################
## Class DownloadDialog2
###########################################################################

class DownloadDialog2 ( wx.Dialog ):
	
	def __init__( self, parent ):
		wx.Dialog.__init__ ( self, parent, id = wx.ID_ANY, title = u"Puzzle Downloader", pos = wx.DefaultPosition, size = wx.DefaultSize, style = wx.DEFAULT_DIALOG_STYLE|wx.RESIZE_BORDER )
		
		self.SetSizeHintsSz( wx.DefaultSize, wx.DefaultSize )
		
		bSizer16 = wx.BoxSizer( wx.HORIZONTAL )
		
		self.tree = wx.TreeCtrl( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TR_FULL_ROW_HIGHLIGHT|wx.TR_HIDE_ROOT|wx.TR_NO_BUTTONS|wx.TR_NO_LINES|wx.TR_SINGLE )
		bSizer16.Add( self.tree, 0, wx.ALL|wx.EXPAND, 5 )
		
		bSizer20 = wx.BoxSizer( wx.VERTICAL )
		
		self.scroller = wx.ScrolledWindow( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.DOUBLE_BORDER|wx.HSCROLL|wx.VSCROLL )
		self.scroller.SetScrollRate( 10, 10 )
		bSizer8 = wx.BoxSizer( wx.VERTICAL )
		
		
		self.scroller.SetSizer( bSizer8 )
		self.scroller.Layout()
		bSizer8.Fit( self.scroller )
		bSizer20.Add( self.scroller, 1, wx.EXPAND |wx.ALL, 5 )
		
		
		bSizer16.Add( bSizer20, 1, wx.EXPAND, 5 )
		
		
		self.SetSizer( bSizer16 )
		self.Layout()
		bSizer16.Fit( self )
		
		self.Centre( wx.BOTH )
		
		# Connect Events
		self.tree.Bind( wx.EVT_TREE_SEL_CHANGED, self.OnTreeSelect )
	
	def __del__( self ):
		pass
	
	
	# Virtual event handlers, overide them in your derived class
	def OnTreeSelect( self, event ):
		event.Skip()
	

###########################################################################
## Class PreferencesPanel
###########################################################################

class PreferencesPanel ( wx.Panel ):
	
	def __init__( self, parent ):
		wx.Panel.__init__ ( self, parent, id = wx.ID_ANY, pos = wx.DefaultPosition, size = wx.Size( 500,300 ), style = wx.TAB_TRAVERSAL )
		
		bSizer19 = wx.BoxSizer( wx.VERTICAL )
		
		bSizer231 = wx.BoxSizer( wx.HORIZONTAL )
		
		self.source_list = CheckListCtrl( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.LC_REPORT|wx.LC_SINGLE_SEL )
		bSizer231.Add( self.source_list, 1, wx.ALL|wx.EXPAND, 5 )
		
		bSizer24 = wx.BoxSizer( wx.VERTICAL )
		
		self.new_button = wx.Button( self, wx.ID_ANY, u"&New", wx.DefaultPosition, wx.DefaultSize, 0 )
		bSizer24.Add( self.new_button, 0, wx.ALL, 5 )
		
		self.edit_button = wx.Button( self, wx.ID_ANY, u"&Edit", wx.DefaultPosition, wx.DefaultSize, 0 )
		bSizer24.Add( self.edit_button, 0, wx.ALL, 5 )
		
		self.remove_button = wx.Button( self, wx.ID_ANY, u"&Remove", wx.DefaultPosition, wx.DefaultSize, 0 )
		bSizer24.Add( self.remove_button, 0, wx.ALL, 5 )
		
		
		bSizer231.Add( bSizer24, 0, wx.EXPAND, 5 )
		
		
		bSizer19.Add( bSizer231, 1, wx.EXPAND, 5 )
		
		bSizer20 = wx.BoxSizer( wx.VERTICAL )
		
		bSizer21 = wx.BoxSizer( wx.HORIZONTAL )
		
		self.m_staticText3 = wx.StaticText( self, wx.ID_ANY, u"Puzzle directory", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText3.Wrap( -1 )
		bSizer21.Add( self.m_staticText3, 0, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 5 )
		
		self.puzzle_directory = wx.DirPickerCtrl( self, wx.ID_ANY, wx.EmptyString, u"Select a folder", wx.DefaultPosition, wx.DefaultSize, wx.DIRP_DEFAULT_STYLE|wx.DIRP_DIR_MUST_EXIST )
		bSizer21.Add( self.puzzle_directory, 1, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		
		bSizer20.Add( bSizer21, 0, wx.EXPAND, 5 )
		
		bSizer23 = wx.BoxSizer( wx.HORIZONTAL )
		
		self.separate_directories = wx.CheckBox( self, wx.ID_ANY, u"Download to separate directories (by puzzle)", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.separate_directories.SetValue(True) 
		bSizer23.Add( self.separate_directories, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )
		
		
		bSizer23.AddSpacer( ( 0, 0), 1, wx.EXPAND, 5 )
		
		self.m_staticText4 = wx.StaticText( self, wx.ID_ANY, u"Auto download", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText4.Wrap( -1 )
		bSizer23.Add( self.m_staticText4, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.auto_download = wx.SpinCtrl( self, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.Size( 50,-1 ), wx.SP_ARROW_KEYS, 0, 10, 0 )
		bSizer23.Add( self.auto_download, 0, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT|wx.LEFT, 5 )
		
		self.m_staticText6 = wx.StaticText( self, wx.ID_ANY, u"days of puzzles", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText6.Wrap( -1 )
		bSizer23.Add( self.m_staticText6, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		
		bSizer20.Add( bSizer23, 0, wx.EXPAND|wx.TOP, 5 )
		
		
		bSizer19.Add( bSizer20, 0, wx.EXPAND|wx.ALL, 5 )
		
		
		self.SetSizer( bSizer19 )
		self.Layout()
		
		# Connect Events
		self.source_list.Bind( wx.EVT_LIST_ITEM_ACTIVATED, self.OnEditSource )
		self.source_list.Bind( wx.EVT_LIST_ITEM_SELECTED, self.OnSourceSelected )
		self.new_button.Bind( wx.EVT_BUTTON, self.OnNewSource )
		self.edit_button.Bind( wx.EVT_BUTTON, self.OnEditSource )
		self.remove_button.Bind( wx.EVT_BUTTON, self.OnRemoveSource )
	
	def __del__( self ):
		pass
	
	
	# Virtual event handlers, overide them in your derived class
	def OnEditSource( self, event ):
		event.Skip()
	
	def OnSourceSelected( self, event ):
		event.Skip()
	
	def OnNewSource( self, event ):
		event.Skip()
	
	
	def OnRemoveSource( self, event ):
		event.Skip()
	

###########################################################################
## Class PuzzleSourceDialog
###########################################################################

class PuzzleSourceDialog ( wx.Dialog ):
	
	def __init__( self, parent ):
		wx.Dialog.__init__ ( self, parent, id = wx.ID_ANY, title = u"Edit Puzzle Source", pos = wx.DefaultPosition, size = wx.DefaultSize, style = wx.DEFAULT_DIALOG_STYLE )
		
		self.SetSizeHintsSz( wx.DefaultSize, wx.DefaultSize )
		
		bSizer25 = wx.BoxSizer( wx.VERTICAL )
		
		self.notebook = wx.Notebook( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_panel3 = wx.Panel( self.notebook, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		bSizer26 = wx.BoxSizer( wx.VERTICAL )
		
		fgSizer3 = wx.FlexGridSizer( 0, 2, 5, 5 )
		fgSizer3.AddGrowableCol( 1 )
		fgSizer3.SetFlexibleDirection( wx.BOTH )
		fgSizer3.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText6 = wx.StaticText( self.m_panel3, wx.ID_ANY, u"Name:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText6.Wrap( -1 )
		fgSizer3.Add( self.m_staticText6, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.name = wx.TextCtrl( self.m_panel3, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer3.Add( self.name, 0, wx.EXPAND, 5 )
		
		self.m_staticText7 = wx.StaticText( self.m_panel3, wx.ID_ANY, u"URL:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText7.Wrap( -1 )
		fgSizer3.Add( self.m_staticText7, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.url = wx.TextCtrl( self.m_panel3, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		self.url.SetToolTipString( u"Full puzzle URL.  Use date format codes." )
		
		fgSizer3.Add( self.url, 0, wx.EXPAND, 5 )
		
		self.m_staticText8 = wx.StaticText( self.m_panel3, wx.ID_ANY, u"Local filename:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText8.Wrap( -1 )
		fgSizer3.Add( self.m_staticText8, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.filename = wx.TextCtrl( self.m_panel3, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		self.filename.SetToolTipString( u"Filename of the downloaded puzzle.  Use date format codes." )
		
		fgSizer3.Add( self.filename, 0, wx.EXPAND, 5 )
		
		self.m_staticText81 = wx.StaticText( self.m_panel3, wx.ID_ANY, u"Puzzle directory:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText81.Wrap( -1 )
		fgSizer3.Add( self.m_staticText81, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.directoryname = wx.TextCtrl( self.m_panel3, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		self.directoryname.SetToolTipString( u"Name of the subdirectory to save puzzles in.  Defaults to the source name." )
		
		fgSizer3.Add( self.directoryname, 0, wx.EXPAND, 5 )
		
		
		bSizer26.Add( fgSizer3, 1, wx.EXPAND|wx.TOP|wx.RIGHT|wx.LEFT, 5 )
		
		sbSizer1 = wx.StaticBoxSizer( wx.StaticBox( self.m_panel3, wx.ID_ANY, u"Days available" ), wx.HORIZONTAL )
		
		self.day1 = wx.CheckBox( sbSizer1.GetStaticBox(), wx.ID_ANY, u"Mon", wx.DefaultPosition, wx.DefaultSize, 0 )
		sbSizer1.Add( self.day1, 0, wx.ALL, 5 )
		
		self.day2 = wx.CheckBox( sbSizer1.GetStaticBox(), wx.ID_ANY, u"Tue", wx.DefaultPosition, wx.DefaultSize, 0 )
		sbSizer1.Add( self.day2, 0, wx.ALL, 5 )
		
		self.day3 = wx.CheckBox( sbSizer1.GetStaticBox(), wx.ID_ANY, u"Wed", wx.DefaultPosition, wx.DefaultSize, 0 )
		sbSizer1.Add( self.day3, 0, wx.ALL, 5 )
		
		self.day4 = wx.CheckBox( sbSizer1.GetStaticBox(), wx.ID_ANY, u"Thu", wx.DefaultPosition, wx.DefaultSize, 0 )
		sbSizer1.Add( self.day4, 0, wx.ALL, 5 )
		
		self.day5 = wx.CheckBox( sbSizer1.GetStaticBox(), wx.ID_ANY, u"Fri", wx.DefaultPosition, wx.DefaultSize, 0 )
		sbSizer1.Add( self.day5, 0, wx.ALL, 5 )
		
		self.day6 = wx.CheckBox( sbSizer1.GetStaticBox(), wx.ID_ANY, u"Sat", wx.DefaultPosition, wx.DefaultSize, 0 )
		sbSizer1.Add( self.day6, 0, wx.ALL, 5 )
		
		self.day7 = wx.CheckBox( sbSizer1.GetStaticBox(), wx.ID_ANY, u"Sun", wx.DefaultPosition, wx.DefaultSize, 0 )
		sbSizer1.Add( self.day7, 0, wx.ALL, 5 )
		
		
		bSizer26.Add( sbSizer1, 0, wx.EXPAND|wx.ALL, 5 )
		
		self.m_staticText122 = wx.StaticText( self.m_panel3, wx.ID_ANY, u"Date format codes for puzzle URL and filename:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText122.Wrap( -1 )
		self.m_staticText122.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, False, wx.EmptyString ) )
		
		bSizer26.Add( self.m_staticText122, 0, wx.TOP|wx.RIGHT|wx.LEFT, 5 )
		
		bSizer27 = wx.BoxSizer( wx.HORIZONTAL )
		
		self.m_staticText121 = wx.StaticText( self.m_panel3, wx.ID_ANY, u"%Y\n%y\n%m\n%d", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText121.Wrap( -1 )
		bSizer27.Add( self.m_staticText121, 0, wx.ALL, 5 )
		
		self.m_staticText12 = wx.StaticText( self.m_panel3, wx.ID_ANY, u"Year (4-digit)\nYear (2-digit)\nMonth\nDay", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText12.Wrap( -1 )
		bSizer27.Add( self.m_staticText12, 0, wx.EXPAND|wx.ALL, 5 )
		
		
		bSizer27.AddSpacer( ( 30, 0), 1, wx.EXPAND, 20 )
		
		self.m_staticText123 = wx.StaticText( self.m_panel3, wx.ID_ANY, u"%A\n%a\n%B\n%b", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText123.Wrap( -1 )
		bSizer27.Add( self.m_staticText123, 0, wx.ALL, 5 )
		
		self.m_staticText1231 = wx.StaticText( self.m_panel3, wx.ID_ANY, u"Weekday name (full)\nWeekday name (abbr)\nMonth name (full)\nMonth name (abbr)", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText1231.Wrap( -1 )
		bSizer27.Add( self.m_staticText1231, 0, wx.ALL, 5 )
		
		
		bSizer26.Add( bSizer27, 0, wx.ALIGN_CENTER_HORIZONTAL, 5 )
		
		
		self.m_panel3.SetSizer( bSizer26 )
		self.m_panel3.Layout()
		bSizer26.Fit( self.m_panel3 )
		self.notebook.AddPage( self.m_panel3, u"Basic Info", True )
		self.m_panel4 = wx.Panel( self.notebook, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		bSizer28 = wx.BoxSizer( wx.VERTICAL )
		
		bSizer29 = wx.BoxSizer( wx.VERTICAL )
		
		self.auth_required = wx.CheckBox( self.m_panel4, wx.ID_ANY, u"Authentication required", wx.DefaultPosition, wx.DefaultSize, 0 )
		bSizer29.Add( self.auth_required, 0, wx.TOP|wx.BOTTOM, 5 )
		
		fgSizer31 = wx.FlexGridSizer( 0, 2, 5, 5 )
		fgSizer31.AddGrowableCol( 1 )
		fgSizer31.SetFlexibleDirection( wx.BOTH )
		fgSizer31.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText61 = wx.StaticText( self.m_panel4, wx.ID_ANY, u"Login URL:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText61.Wrap( -1 )
		fgSizer31.Add( self.m_staticText61, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.auth_url = wx.TextCtrl( self.m_panel4, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		self.auth_url.SetToolTipString( u"URL that contains an HTML login form." )
		
		fgSizer31.Add( self.auth_url, 0, wx.EXPAND, 5 )
		
		self.m_staticText71 = wx.StaticText( self.m_panel4, wx.ID_ANY, u"Username field:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText71.Wrap( -1 )
		fgSizer31.Add( self.m_staticText71, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.auth_user_id = wx.TextCtrl( self.m_panel4, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		self.auth_user_id.SetToolTipString( u"HTML name attribute of the username input field." )
		
		fgSizer31.Add( self.auth_user_id, 0, wx.EXPAND, 5 )
		
		self.m_staticText82 = wx.StaticText( self.m_panel4, wx.ID_ANY, u"Password field:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText82.Wrap( -1 )
		fgSizer31.Add( self.m_staticText82, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.auth_password_id = wx.TextCtrl( self.m_panel4, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		self.auth_password_id.SetToolTipString( u"HTML name attribute of the password input field." )
		
		fgSizer31.Add( self.auth_password_id, 0, wx.EXPAND, 5 )
		
		
		bSizer29.Add( fgSizer31, 1, wx.EXPAND|wx.LEFT, 15 )
		
		self.m_staticText37 = wx.StaticText( self.m_panel4, wx.ID_ANY, u"Note: Authentication may not work for every source.  Login is attempted via an HTML form.  Any cookies set after a successful login are saved and used to download puzzles.", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText37.Wrap( 325 )
		self.m_staticText37.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_ITALIC, wx.FONTWEIGHT_NORMAL, False, wx.EmptyString ) )
		
		bSizer29.Add( self.m_staticText37, 0, wx.ALL|wx.ALIGN_CENTER_HORIZONTAL, 5 )
		
		
		bSizer28.Add( bSizer29, 1, wx.EXPAND|wx.ALL, 5 )
		
		
		self.m_panel4.SetSizer( bSizer28 )
		self.m_panel4.Layout()
		bSizer28.Fit( self.m_panel4 )
		self.notebook.AddPage( self.m_panel4, u"Authentication", False )
		self.m_panel6 = wx.Panel( self.notebook, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		bSizer30 = wx.BoxSizer( wx.VERTICAL )
		
		self.custom_func_message = wx.StaticText( self.m_panel6, wx.ID_ANY, u"This source uses a custom download function!  Not all changes you make in this dialog may have the desired effect.  Click to view the function.", wx.DefaultPosition, wx.DefaultSize, wx.ALIGN_CENTRE )
		self.custom_func_message.Wrap( 250 )
		self.custom_func_message.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD, False, wx.EmptyString ) )
		self.custom_func_message.SetBackgroundColour( wx.Colour( 255, 255, 0 ) )
		self.custom_func_message.Hide()
		
		bSizer30.Add( self.custom_func_message, 0, wx.ALL|wx.EXPAND, 5 )
		
		self.not_puzzle = wx.CheckBox( self.m_panel6, wx.ID_ANY, u"Skip puzzle validation", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.not_puzzle.SetToolTipString( u"Check to allow downloading files that are not XWord puzzles (e.g. PDFs)" )
		
		bSizer30.Add( self.not_puzzle, 0, wx.ALL, 5 )
		
		self.m_staticText22 = wx.StaticText( self.m_panel6, wx.ID_ANY, u"TODO: cURL options", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText22.Wrap( -1 )
		bSizer30.Add( self.m_staticText22, 0, wx.ALL, 5 )
		
		self.curl_opts = ControlGrid( self.m_panel6, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		bSizer30.Add( self.curl_opts, 1, wx.EXPAND |wx.ALL, 5 )
		
		
		self.m_panel6.SetSizer( bSizer30 )
		self.m_panel6.Layout()
		bSizer30.Fit( self.m_panel6 )
		self.notebook.AddPage( self.m_panel6, u"Advanced", False )
		self.custom_func_panel = wx.Panel( self.notebook, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		bSizer34 = wx.BoxSizer( wx.VERTICAL )
		
		self.m_staticText21 = wx.StaticText( self.custom_func_panel, wx.ID_ANY, u"Don't edit this function unless you know what you're doing.", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText21.Wrap( -1 )
		self.m_staticText21.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD, False, wx.EmptyString ) )
		
		bSizer34.Add( self.m_staticText21, 0, wx.ALL, 5 )
		
		self.m_panel8 = wx.Panel( self.custom_func_panel, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.DOUBLE_BORDER )
		bSizer32 = wx.BoxSizer( wx.VERTICAL )
		
		self.m_staticText40 = wx.StaticText( self.m_panel8, wx.ID_ANY, u"function(puzzle)", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText40.Wrap( -1 )
		self.m_staticText40.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_TELETYPE, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, False, wx.EmptyString ) )
		self.m_staticText40.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_GRAYTEXT ) )
		
		bSizer32.Add( self.m_staticText40, 0, wx.EXPAND, 5 )
		
		self.func = wx.TextCtrl( self.m_panel8, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, wx.TE_DONTWRAP|wx.TE_MULTILINE|wx.NO_BORDER )
		self.func.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_TELETYPE, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, False, wx.EmptyString ) )
		
		bSizer32.Add( self.func, 1, wx.EXPAND, 5 )
		
		
		self.m_panel8.SetSizer( bSizer32 )
		self.m_panel8.Layout()
		bSizer32.Fit( self.m_panel8 )
		bSizer34.Add( self.m_panel8, 1, wx.EXPAND |wx.ALL, 5 )
		
		
		self.custom_func_panel.SetSizer( bSizer34 )
		self.custom_func_panel.Layout()
		bSizer34.Fit( self.custom_func_panel )
		self.notebook.AddPage( self.custom_func_panel, u"Custom Function", False )
		
		bSizer25.Add( self.notebook, 1, wx.EXPAND|wx.ALL, 5 )
		
		m_sdbSizer2 = wx.StdDialogButtonSizer()
		self.m_sdbSizer2OK = wx.Button( self, wx.ID_OK )
		m_sdbSizer2.AddButton( self.m_sdbSizer2OK )
		self.m_sdbSizer2Cancel = wx.Button( self, wx.ID_CANCEL )
		m_sdbSizer2.AddButton( self.m_sdbSizer2Cancel )
		m_sdbSizer2.Realize();
		
		bSizer25.Add( m_sdbSizer2, 0, wx.EXPAND|wx.ALL, 5 )
		
		
		self.SetSizer( bSizer25 )
		self.Layout()
		bSizer25.Fit( self )
		
		self.Centre( wx.BOTH )
		
		# Connect Events
		self.name.Bind( wx.EVT_TEXT, self.OnNameChanged )
		self.directoryname.Bind( wx.EVT_TEXT, self.OnDirectoryChanged )
		self.auth_required.Bind( wx.EVT_CHECKBOX, self.OnAuthRequired )
		self.custom_func_message.Bind( wx.EVT_LEFT_UP, self.OnShowCustomFunction )
		self.m_sdbSizer2OK.Bind( wx.EVT_BUTTON, self.OnOK )
	
	def __del__( self ):
		pass
	
	
	# Virtual event handlers, overide them in your derived class
	def OnNameChanged( self, event ):
		event.Skip()
	
	def OnDirectoryChanged( self, event ):
		event.Skip()
	
	def OnAuthRequired( self, event ):
		event.Skip()
	
	def OnShowCustomFunction( self, event ):
		event.Skip()
	
	def OnOK( self, event ):
		event.Skip()
	

