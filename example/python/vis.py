import bgl
import wx
import wx.lib.ogl as ogl
# import wx.lib.masked
import os
import random
import sys

wildcard = "GraphViz (*.dot)|*.dot|"   \
           "All files (*.*)|*.*"

def path_to_title(path):
  title = path
  dot_idx = title.rfind('.')
  if dot_idx != -1: title = title[:dot_idx]
  slash_idx = title.rfind('/')
  if slash_idx != -1: title = title[slash_idx+1:]
  slash_idx = title.rfind('\\')
  if slash_idx != -1: title = title[slash_idx+1:]
  return title

class GraphCanvas(ogl.ShapeCanvas):
    def __init__(self, parent):
        ogl.ShapeCanvas.__init__(self, parent)

        maxWidth  = 400
        maxHeight = 400
        self.SetScrollbars(20, 20, maxWidth/20, maxHeight/20)

    def set_graph(self, graph, position_map, property_maps):
        self.diagram = ogl.Diagram()
        self.SetDiagram(self.diagram)
        self.diagram.SetCanvas(self)

        self.graph = graph
        self.position_map = position_map
        self.property_maps = property_maps
        self.vertex_position_rect = self.compute_rect()
        self.vertex_to_shape = graph.get_vertex_object_map("__stored_shape")
        self.edge_to_shape = graph.get_edge_object_map("__stored_shape")
        self.shape_to_vertex = {}

        for v in graph.vertices:
           self.add_vertex(v)

        for e in graph.edges:
           self.add_edge(e)

        self.Refresh()

    def compute_rect(self):
        pos = self.position_map
        if self.graph.num_vertices() == 0:
            return (-50, -50, 50, 50)
        else:
          left, top, right, bottom = 0, 0, 0, 0
          for v in self.graph.vertices:
              if pos[v].x < left: left = pos[v].x
              if pos[v].y < top: top = pos[v].y
              if pos[v].x > right: right = pos[v].x
              if pos[v].y > bottom: bottom = pos[v].y
          return (left, top, right, bottom)

    def update_layout(self):
        self.vertex_position_rect = self.compute_rect()
        dc =  wx.ClientDC(self)
        self.PrepareDC(dc)
        for v in self.graph.vertices:
            shape = self.vertex_to_shape[v]
            x, y = self.vertex_position_to_shape_position(v)
            shape.Move(dc, x, y, False)
        self.Refresh()

    def vertex_position_to_shape_position(self, vertex):
        (width, height) = self.GetVirtualSize()
        width = width - 20
        height = height - 20
        (pos_width, pos_height) = (self.vertex_position_rect[2] 
                                   - self.vertex_position_rect[0],
                                   self.vertex_position_rect[3] 
                                   - self.vertex_position_rect[1])
        return ((self.position_map[vertex].x - self.vertex_position_rect[0]) 
                / pos_width * width + 10,
                (self.position_map[vertex].y - self.vertex_position_rect[1])
                / pos_height * height + 10);

    def translate_color(self, color):
        if color=="black": return wx.BLACK
        elif color=="blue": return wx.BLUE
        elif color=="red": return wx.RED
        elif color=="green": return wx.GREEN
        else: return wx.BLACK

    def add_vertex(self, vertex):
        shape = self.CreateVertex(vertex)
        shape.SetDraggable(True, True)
        shape.SetCanvas(self)
        x, y = self.vertex_position_to_shape_position(vertex)
        shape.SetX(x)
        shape.SetY(y)
        shape.SetPen(self.VertexPen(vertex))
        shape.SetBrush(self.VertexBrush(vertex))
        s = self.VertexLabel(vertex)
        if s != "": shape.AddText(s)
        self.diagram.AddShape(shape)
        shape.Show(True)
        self.vertex_to_shape[vertex] = shape
        self.shape_to_vertex[shape] = vertex
        
        evthandler = VertexEventHandler(self)
        evthandler.SetShape(shape)
        evthandler.SetPreviousHandler(shape.GetEventHandler())
        shape.SetEventHandler(evthandler)

        return shape;

    def CreateVertex(self, vertex):
        return ogl.CircleShape(20)

    def VertexPen(self, vertex):
        thickness = 1
        color = wx.BLACK
        if "vertex.border.thickness" in self.property_maps:
            thickness = self.property_maps["vertex.border.thickness"][vertex]
            if "vertex.border.color" in self.property_maps:
                color_text = self.property_maps["vertex.border.color"][vertex]
                color = translate_color(color_text)
        return wx.Pen(color, thickness)

    def VertexBrush(self, vertex):
      return wx.GREEN_BRUSH

    def VertexLabel(self, vertex):
      if "vertex.label" in self.property_maps:
        return self.property_maps["vertex.label"][vertex]
      else:
        return ""

    def VertexShape(self, vertex):
      return self.vertex_to_shape[vertex]

    def add_edge(self, edge):
        (u, v) = (self.graph.source(edge), self.graph.target(edge))
        line = ogl.LineShape()
        line.SetCanvas(self)
        line.SetPen(wx.BLACK_PEN)
        line.SetBrush(wx.BLACK_BRUSH)
        if self.graph.is_directed(): line.AddArrow(ogl.ARROW_ARROW)
        if "edge.label" in self.property_maps:
            label = str(self.property_maps["edge.label"][edge])
            line.AddText(label)
        line.MakeLineControlPoints(2)
        self.vertex_to_shape[u].AddLine(line, self.vertex_to_shape[v])
        self.diagram.AddShape(line)
        line.Show(True)
        self.edge_to_shape[edge] = line
        return line

    def EdgeShape(self, edge):
      return self.edge_to_shape[edge]

    def set_vertex_colors_from_components(self, component_map):
        brushes = {}
        for v in self.graph.vertices:
            shape = self.vertex_to_shape[v]
            comp = component_map[v]
            if not comp in brushes:
                brushes[comp] = wx.Brush(wx.Color(random.randint(0, 200),
                                                  random.randint(0, 200),
                                                  random.randint(0, 200)))
            shape.SetBrush(brushes[comp])
        self.Refresh()
        
    def set_edge_colors_from_components(self, component_map):
        pens = {}
        for e in self.graph.edges:
            shape = self.edge_to_shape[e]
            comp = component_map[e]
            if not comp in pens:
                pens[comp] = wx.Pen(wx.Color(random.randint(0, 200),
                                             random.randint(0, 200),
                                             random.randint(0, 200)),
                                    1)
            shape.SetPen(pens[comp])
        self.Refresh()

    def default_property_maps(self, graph, node_id = "node_id"):
        maps = {}
        if graph.has_vertex_map("label"): 
            maps["vertex.label"] = graph.get_vertex_string_map("label")
        elif graph.has_vertex_map(node_id):
            maps["vertex.label"] = graph.get_vertex_string_map(node_id)
        if graph.has_edge_map("label"):
            maps["edge.label"] = graph.get_edge_string_map("label")
        elif graph.has_edge_map("weight"):
            maps["edge.label"] = graph.get_edge_double_map("weight")
        return maps

class VertexEventHandler(ogl.ShapeEvtHandler):
  def __init__(self, graphwin):
    ogl.ShapeEvtHandler.__init__(self)
    self.graphwin = graphwin

  def OnEndDragLeft(self, x, y, keys=0, attachment=0):
    shape = self.GetShape()   
    if shape.Selected():
      vertex = self.graphwin.shape_to_vertex[shape]
      self.graphwin.position_map[vertex].x = x
      self.graphwin.position_map[vertex].y = y
    ogl.ShapeEvtHandler.OnEndDragLeft(self, x, y, keys, attachment)

class ErdosRenyiDialog(wx.Dialog):
    def __init__(
            self, parent, ID, title, size=wx.DefaultSize, pos=wx.DefaultPosition, 
            style=wx.DEFAULT_DIALOG_STYLE
            ):
        wx.Dialog.__init__(self, parent, ID, title, pos, size, style, 
                           "Erdos-Renyi Generator")

        sizer = wx.BoxSizer(wx.VERTICAL)

        box = wx.BoxSizer(wx.HORIZONTAL)

        box.Add(wx.StaticText(self, -1, "Number of vertices (n)"), 0, 
                wx.ALIGN_CENTRE|wx.ALL, 5)

        num_vertices_spin = wx.SpinCtrl(self, -1, "", (30, 50))
        num_vertices_spin.SetRange(0,10000000)
        num_vertices_spin.SetValue(10)
        box.Add(num_vertices_spin, 1, wx.ALIGN_CENTRE|wx.ALL, 5)
        self.num_vertices_spin = num_vertices_spin
        sizer.Add(box, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        box = wx.BoxSizer(wx.HORIZONTAL)

        box.Add(wx.StaticText(self, -1, "Edge probability (%)"), 0, 
                wx.ALIGN_CENTRE|wx.ALL, 5)

        # This is better than what we're currently doing with a SpinCtrl, but
        # it makes the program unstable (?)
#        probability_ctrl = wx.lib.masked.numctrl.NumCtrl(self, value = 0.2,
#                                                         integerWidth = 1,
#                                                         fractionWidth = 3,
#                                                         allowNegative = False,
#                                                         min = 0.0, max = 1.0)

        probability_ctrl = wx.SpinCtrl(self, -1, "", (30, 50))
        probability_ctrl.SetRange(0,100)
        probability_ctrl.SetValue(20)
        self.probability_ctrl = probability_ctrl
        box.Add(probability_ctrl, 1, wx.ALIGN_CENTRE|wx.ALL, 5)
        sizer.Add(box, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        box = wx.BoxSizer(wx.HORIZONTAL)
        box.Add(wx.StaticText(self, -1, "Random seed"), 0, 
                wx.ALIGN_CENTRE|wx.ALL, 5)
        
        seed_ctrl = wx.SpinCtrl(self, -1, "", (30, 50))
        seed_ctrl.SetRange(1, sys.maxint)
        seed_ctrl.SetValue(random.randint(1, sys.maxint))
        self.seed_ctrl = seed_ctrl
        box.Add(seed_ctrl, 1, wx.ALIGN_CENTRE|wx.ALL, 5)

        sizer.Add(box, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        line = wx.StaticLine(self, -1, size=(20,-1), style=wx.LI_HORIZONTAL)
        sizer.Add(line, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.RIGHT|wx.TOP, 5)

        box = wx.BoxSizer(wx.HORIZONTAL)

        btn = wx.Button(self, wx.ID_OK, " Generate! ")
        btn.SetDefault()
        box.Add(btn, 0, wx.ALIGN_CENTRE|wx.ALL, 5)

        btn = wx.Button(self, wx.ID_CANCEL, " Cancel ")
        box.Add(btn, 0, wx.ALIGN_CENTRE|wx.ALL, 5)

        sizer.Add(box, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        self.SetSizer(sizer)
        self.SetAutoLayout(True)
        sizer.Fit(self)

    def GetNumVertices(self):
        return self.num_vertices_spin.GetValue()

    def GetProbability(self):
        return float(self.probability_ctrl.GetValue())

    def GetRandomSeed(self):
        return int(self.seed_ctrl.GetValue())

class PLODDialog(wx.Dialog):
    def __init__(
            self, parent, ID, title, size=wx.DefaultSize, pos=wx.DefaultPosition, 
            style=wx.DEFAULT_DIALOG_STYLE
            ):
        wx.Dialog.__init__(self, parent, ID, title, pos, size, style)

        sizer = wx.BoxSizer(wx.VERTICAL)

        box = wx.BoxSizer(wx.HORIZONTAL)

        box.Add(wx.StaticText(self, -1, "Number of vertices (n)"), 0, 
                wx.ALIGN_CENTRE|wx.ALL, 5)

        num_vertices_spin = wx.SpinCtrl(self, -1, "", (30, 50))
        num_vertices_spin.SetRange(0,10000000)
        num_vertices_spin.SetValue(10)
        box.Add(num_vertices_spin, 1, wx.ALIGN_CENTRE|wx.ALL, 5)
        self.num_vertices_spin = num_vertices_spin
        sizer.Add(box, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        # Alpha
        box = wx.BoxSizer(wx.HORIZONTAL)
        box.Add(wx.StaticText(self, -1, "Alpha"), 0, 
                wx.ALIGN_CENTRE|wx.ALL, 5)
        alpha_ctrl = wx.TextCtrl(self, -1, "2.75", (30, 50))
        self.alpha_ctrl = alpha_ctrl
        box.Add(alpha_ctrl, 1, wx.ALIGN_CENTRE|wx.ALL, 5)
        sizer.Add(box, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)


        # Beta
        box = wx.BoxSizer(wx.HORIZONTAL)
        box.Add(wx.StaticText(self, -1, "Beta"), 0, 
                wx.ALIGN_CENTRE|wx.ALL, 5)
        beta_ctrl = wx.SpinCtrl(self, -1, "", (30, 50))
        beta_ctrl.SetRange(1, sys.maxint)
        beta_ctrl.SetValue(300)
        self.beta_ctrl = beta_ctrl
        box.Add(beta_ctrl, 1, wx.ALIGN_CENTRE|wx.ALL, 5)
        sizer.Add(box, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)


        box = wx.BoxSizer(wx.HORIZONTAL)
        box.Add(wx.StaticText(self, -1, "Random seed"), 0, 
                wx.ALIGN_CENTRE|wx.ALL, 5)
        
        seed_ctrl = wx.SpinCtrl(self, -1, "", (30, 50))
        seed_ctrl.SetRange(1, sys.maxint)
        seed_ctrl.SetValue(random.randint(1, sys.maxint))
        self.seed_ctrl = seed_ctrl
        box.Add(seed_ctrl, 1, wx.ALIGN_CENTRE|wx.ALL, 5)

        sizer.Add(box, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        line = wx.StaticLine(self, -1, size=(20,-1), style=wx.LI_HORIZONTAL)
        sizer.Add(line, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.RIGHT|wx.TOP, 5)

        box = wx.BoxSizer(wx.HORIZONTAL)

        btn = wx.Button(self, wx.ID_OK, " Generate! ")
        btn.SetDefault()
        box.Add(btn, 0, wx.ALIGN_CENTRE|wx.ALL, 5)

        btn = wx.Button(self, wx.ID_CANCEL, " Cancel ")
        box.Add(btn, 0, wx.ALIGN_CENTRE|wx.ALL, 5)

        sizer.Add(box, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        self.SetSizer(sizer)
        self.SetAutoLayout(True)
        sizer.Fit(self)

    def GetNumVertices(self):
        return self.num_vertices_spin.GetValue()

    def GetAlpha(self):
        return float(self.alpha_ctrl.GetValue())

    def GetBeta(self):
        return float(self.beta_ctrl.GetValue())

    def GetRandomSeed(self):
        return int(self.seed_ctrl.GetValue())

class SmallWorldDialog(wx.Dialog):
    def __init__(
            self, parent, ID, title, size=wx.DefaultSize, pos=wx.DefaultPosition, 
            style=wx.DEFAULT_DIALOG_STYLE
            ):
        wx.Dialog.__init__(self, parent, ID, title, pos, size, style)

        sizer = wx.BoxSizer(wx.VERTICAL)

        box = wx.BoxSizer(wx.HORIZONTAL)

        box.Add(wx.StaticText(self, -1, "Number of vertices (n)"), 0, 
                wx.ALIGN_CENTRE|wx.ALL, 5)

        num_vertices_spin = wx.SpinCtrl(self, -1, "", (30, 50))
        num_vertices_spin.SetRange(0,10000000)
        num_vertices_spin.SetValue(10)
        box.Add(num_vertices_spin, 1, wx.ALIGN_CENTRE|wx.ALL, 5)
        self.num_vertices_spin = num_vertices_spin
        sizer.Add(box, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        # Number of neighbors
        box = wx.BoxSizer(wx.HORIZONTAL)

        box.Add(wx.StaticText(self, -1, "Number of neighbors (k)"), 0, 
                wx.ALIGN_CENTRE|wx.ALL, 5)

        num_neighbors_spin = wx.SpinCtrl(self, -1, "", (30, 50))
        num_neighbors_spin.SetRange(0,10000000)
        num_neighbors_spin.SetValue(4)
        box.Add(num_neighbors_spin, 1, wx.ALIGN_CENTRE|wx.ALL, 5)
        self.num_neighbors_spin = num_neighbors_spin
        sizer.Add(box, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        box = wx.BoxSizer(wx.HORIZONTAL)

        box.Add(wx.StaticText(self, -1, "Rewiring probability (%)"), 0, 
                wx.ALIGN_CENTRE|wx.ALL, 5)

        # This is better than what we're currently doing with a SpinCtrl, but
        # it makes the program unstable (?)
#        probability_ctrl = wx.lib.masked.numctrl.NumCtrl(self, value = 0.2,
#                                                         integerWidth = 1,
#                                                         fractionWidth = 3,
#                                                         allowNegative = False,
#                                                         min = 0.0, max = 1.0)

        probability_ctrl = wx.SpinCtrl(self, -1, "", (30, 50))
        probability_ctrl.SetRange(0,100)
        probability_ctrl.SetValue(20)
        self.probability_ctrl = probability_ctrl
        box.Add(probability_ctrl, 1, wx.ALIGN_CENTRE|wx.ALL, 5)
        sizer.Add(box, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        box = wx.BoxSizer(wx.HORIZONTAL)
        box.Add(wx.StaticText(self, -1, "Random seed"), 0, 
                wx.ALIGN_CENTRE|wx.ALL, 5)
        
        seed_ctrl = wx.SpinCtrl(self, -1, "", (30, 50))
        seed_ctrl.SetRange(1, sys.maxint)
        seed_ctrl.SetValue(random.randint(1, sys.maxint))
        self.seed_ctrl = seed_ctrl
        box.Add(seed_ctrl, 1, wx.ALIGN_CENTRE|wx.ALL, 5)

        sizer.Add(box, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        line = wx.StaticLine(self, -1, size=(20,-1), style=wx.LI_HORIZONTAL)
        sizer.Add(line, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.RIGHT|wx.TOP, 5)

        box = wx.BoxSizer(wx.HORIZONTAL)

        btn = wx.Button(self, wx.ID_OK, " Generate! ")
        btn.SetDefault()
        box.Add(btn, 0, wx.ALIGN_CENTRE|wx.ALL, 5)

        btn = wx.Button(self, wx.ID_CANCEL, " Cancel ")
        box.Add(btn, 0, wx.ALIGN_CENTRE|wx.ALL, 5)

        sizer.Add(box, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        self.SetSizer(sizer)
        self.SetAutoLayout(True)
        sizer.Fit(self)

    def GetNumVertices(self):
        return self.num_vertices_spin.GetValue()

    def GetNumNeighbors(self):
        return self.num_neighbors_spin.GetValue()

    def GetProbability(self):
        return float(self.probability_ctrl.GetValue())

    def GetRandomSeed(self):
        return int(self.seed_ctrl.GetValue())

class GraphEditorWindow(wx.Frame):
    def __init__(self, parent, title='BGL Graph Viewer'):
        wx.Frame.__init__(self, parent, -1, title);
        self.canvas = GraphCanvas(self)
        self.CreateMenuBar()
        self.erdos_renyi_dlg = None
        self.plod_dlg = None
        self.small_world_dlg = None
        self.NewGraph(None)

    def CreateMenuBar(self):
        menuBar = wx.MenuBar()

        # File menu
        fileMenu = wx.Menu()

        # New graph menu
        newGraphMenu = wx.Menu()
        newGraphMenu.Append(111, "Empty graph")
        newGraphMenu.Append(112, "Erdos-Renyi graph...")
        newGraphMenu.Append(113, "Power Law Out Degree graph...")
        newGraphMenu.Append(114, "Small-world graph...")
        fileMenu.AppendMenu(110, "&New graph", newGraphMenu)

        fileMenu.Append(120, "&Open graph")
        fileMenu.Append(130, "&Save graph")
        menuBar.Append(fileMenu, "&File")

        # Algorithms menu
        algorithmsMenu = wx.Menu()
        #  - Connected components menu
        ccMenu = wx.Menu();
        ccMenu.Append(201, "Connected Components")
        ccMenu.Append(202, "Strongly-Connected Components")
        ccMenu.Append(203, "Biconnected Components")
        algorithmsMenu.AppendMenu(200, "Connected Components", ccMenu)

        # - Minimum Spanning Tree menu
        mstMenu = wx.Menu();
        mstMenu.Append(212, "Kruskal")
        algorithmsMenu.AppendMenu(210, "Minimum Spanning Tree", mstMenu)

        # Other algorithms...
        algorithmsMenu.Append(221, "Sequential vertex coloring")

        menuBar.Append(algorithmsMenu, "&Algorithms")


        # Layout menu
        layoutMenu = wx.Menu()
        layoutMenu.Append(301, "&Circle layout")
        layoutMenu.Append(302, "&Fruchterman-Reingold layout")
        layoutMenu.Append(303, "&Kamada-Kawai layout")
        menuBar.Append(layoutMenu, "&Layout")

        # File menu events
        self.Bind(wx.EVT_MENU, self.NewGraph, id=111)
        self.Bind(wx.EVT_MENU, self.ErdosRenyiGraph, id=112)
        self.Bind(wx.EVT_MENU, self.PLODGraph, id=113)
        self.Bind(wx.EVT_MENU, self.SmallWorldGraph, id=114)
        self.Bind(wx.EVT_MENU, self.OpenGraph, id=120)
        self.Bind(wx.EVT_MENU, self.SaveGraph, id=130)

        # Algorithms menu events
        self.Bind(wx.EVT_MENU, self.ConnectedComponents, id=201)
        self.Bind(wx.EVT_MENU, self.StrongComponents, id=202)
        self.Bind(wx.EVT_MENU, self.BiconnectedComponents, id=203)
        self.Bind(wx.EVT_MENU, self.KruskalMST, id=212)
        self.Bind(wx.EVT_MENU, self.SequentialVertexColoring, id=221)

        # Layout menu events
        self.Bind(wx.EVT_MENU, self.CircleLayout, id=301)
        self.Bind(wx.EVT_MENU, self.FruchtermanReingoldLayout, id=302)
        self.Bind(wx.EVT_MENU, self.KamadaKawaiLayout, id=303)

        self.SetMenuBar(menuBar)

    def NewGraph(self, event):
        graph = bgl.Graph()
        position_map = graph.get_vertex_point2d_map("position")
        self.canvas.set_graph(graph, position_map, {})
        self.SetTitle("Graph")

    def ErdosRenyiGraph(self, event):
        if not self.erdos_renyi_dlg:
            self.erdos_renyi_dlg = ErdosRenyiDialog(self, -1, 
                                                    "Erdos-Renyi Generator")
        dlg = self.erdos_renyi_dlg
        if dlg.ShowModal() == wx.ID_OK:
            graph = bgl.Graph(bgl.ErdosRenyi(dlg.GetNumVertices(), 
                                             dlg.GetProbability() / 100),
                              dlg.GetRandomSeed())
            position_map = graph.get_vertex_point2d_map("position")
            bgl.circle_graph_layout(graph, position_map, 50)
            self.canvas.set_graph(graph, position_map, {})
            self.SetTitle("Erdos-Renyi Graph (" 
                          + str(dlg.GetNumVertices()) + ", "
                          + str(dlg.GetProbability() / 100) + ")")
    def PLODGraph(self, event):
        if not self.plod_dlg:
            self.plod_dlg = PLODDialog(self, -1, 
                                       "Power Law Out Degree Generator")
        dlg = self.plod_dlg
        if dlg.ShowModal() == wx.ID_OK:
            graph = bgl.Graph(bgl.PowerLawOutDegree(dlg.GetNumVertices(), 
                                                    dlg.GetAlpha(),
                                                    dlg.GetBeta()),
                              dlg.GetRandomSeed())
            position_map = graph.get_vertex_point2d_map("position")
            bgl.circle_graph_layout(graph, position_map, 50)
            self.canvas.set_graph(graph, position_map, {})
            self.SetTitle("Power Law Out Degree Graph (" 
                          + str(dlg.GetNumVertices()) + ", "
                          + str(dlg.GetAlpha()) + ", "
                          + str(dlg.GetBeta()) + ")")

    def SmallWorldGraph(self, event):
        if not self.small_world_dlg:
            self.small_world_dlg = SmallWorldDialog(self, -1, 
                                                    "Small-World Generator")
        dlg = self.small_world_dlg
        if dlg.ShowModal() == wx.ID_OK:
            graph = bgl.Graph(bgl.SmallWorld(dlg.GetNumVertices(), 
                                             dlg.GetNumNeighbors(),
                                             dlg.GetProbability() / 100),
                              dlg.GetRandomSeed())
            position_map = graph.get_vertex_point2d_map("position")
            bgl.circle_graph_layout(graph, position_map, 50)
            self.canvas.set_graph(graph, position_map, {})
            self.SetTitle("Small-World Graph (" 
                          + str(dlg.GetNumVertices()) + ", "
                          + str(dlg.GetNumNeighbors()) + ", "
                          + str(dlg.GetProbability() / 100) + ")")

    def OpenGraph(self, event):
        dlg = wx.FileDialog(
            self, message="Choose a file", defaultDir=os.getcwd(),
            defaultFile="", wildcard=wildcard, style=wx.OPEN | wx.CHANGE_DIR)
        if dlg.ShowModal() == wx.ID_OK:
            path = dlg.GetPath()
            try:
              graph = bgl.Graph(path, bgl.file_kind.graphviz)
            except bgl.directed_graph_error:
              graph = bgl.Digraph(path, bgl.file_kind.graphviz)
            needs_layout = not graph.has_vertex_map("position")
            position_map = graph.get_vertex_point2d_map("position")
            if needs_layout:
                bgl.circle_graph_layout(graph, position_map, 50)
                
            self.canvas.set_graph(graph, position_map,
                                  self.canvas.default_property_maps(graph))

            self.SetTitle(path_to_title(path))

        dlg.Destroy()

    def SaveGraph(self, event):
        dlg = wx.FileDialog(
            self, message="Choose a file", defaultDir=os.getcwd(),
            defaultFile="", wildcard=wildcard, style=wx.SAVE | wx.CHANGE_DIR)
        if dlg.ShowModal() == wx.ID_OK:
            path = dlg.GetPath()
            # TBD: This fails because we can't serialize Python
            # objects. Need to be able to mark some property maps as
            # internal (i.e., don't serialize them).
            self.canvas.graph.write_graphviz(path)
        dlg.Destroy()

    def ConnectedComponents(self, event):
        graph = self.canvas.graph
        component_map = graph.get_vertex_int_map("component")
        bgl.connected_components(graph, component_map)
        self.canvas.set_vertex_colors_from_components(component_map)

    def StrongComponents(self, event):
        graph = self.canvas.graph
        component_map = graph.get_vertex_int_map("component")
        bgl.strong_components(graph, component_map)
        self.canvas.set_vertex_colors_from_components(component_map)
        
    def BiconnectedComponents(self, event):
        graph = self.canvas.graph
        component_map = graph.get_edge_int_map("component")
        art_points = bgl.biconnected_components(graph, component_map)
        for v in art_points:
            self.canvas.VertexShape(v).SetBrush(wx.RED_BRUSH)
        self.canvas.set_edge_colors_from_components(component_map)

    def KruskalMST(self, event):
        graph = self.canvas.graph
        weight_map = graph.get_edge_double_map("weight")
        mst_edges = bgl.kruskal_minimum_spanning_tree(graph, weight_map)
        for e in mst_edges:
            shape = self.canvas.EdgeShape(e)
            shape.SetPen(wx.Pen(shape.GetPen().GetColour(), 3))
        self.canvas.Refresh()

    def SequentialVertexColoring(self, event):
        graph = self.canvas.graph
        color_map = graph.get_vertex_int_map("color")
        bgl.sequential_vertex_coloring(graph, color_map)
        self.canvas.set_vertex_colors_from_components(color_map)

    def CircleLayout(self, event):
        bgl.circle_graph_layout(self.canvas.graph, self.canvas.position_map, 50)
        self.canvas.update_layout()

    def FruchtermanReingoldLayout(self, event):
        bgl.fruchterman_reingold_force_directed_layout(self.canvas.graph, 
                                                       self.canvas.position_map,
                                                       width=100, height=100,
                                                       progressive=True)
        self.canvas.update_layout()

    def KamadaKawaiLayout(self, event):
        bgl.kamada_kawai_spring_layout(self.canvas.graph, 
                                       self.canvas.position_map, side_length=90)
        self.canvas.update_layout()


class GraphDrawApp(wx.App):
    def OnInit(self):
        # This creates some pens and brushes that the OGL library uses.
        # It should be called after the app object has been created, but
        # before OGL is used.
        ogl.OGLInitialize()
        
        self.editor = GraphEditorWindow(None)
        self.editor.Show(True) 
        return True

app = GraphDrawApp(0)
app.MainLoop()
